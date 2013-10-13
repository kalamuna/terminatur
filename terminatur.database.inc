<?php

 /**
 * @file
 *  database.inc: Helper functions to grab data
 */

/**
 * Gets the DB either by pipe or download adn then imports into mysql
 *
 * Validates and builds settings files it applicable.
 */
function _terminatur_data_mysql($site, $destination, $db_user, $db_pass, $db_host, $db_port, $pipe = FALSE) {
  $databases = array();
  if (!$databases = _terminatur_data_get_db_creds($site, $destination, $db_user, $db_pass, $db_host, $db_port)) {
    drush_log(dt("You have entered incorrect MySQL credentials."), 'error');
    return;
  }

  // Try to get the most up to date data with sql-sync-pipe
  if ($pipe) {
    if (function_exists('drush_sql_sync_pipe')) {
      if (!$local_site = _terminatur_aliases_get_local_site($site['machine-name'])) {
        drush_log(dt("You need to have a local site set up to use pipe."), 'error');
        return;
      }
      // Unreap my heart: make sure DB server is awake
      drush_invoke_process("@pantheon." . $site['machine-name'] . "." . $site['env'], 'core-status', array(), array());
      drush_invoke_process('', 'sql-sync-pipe', array("@pantheon." . $site['machine-name'] . "." . $site['env'], "@local." . $site['machine-name'] . TERMINATUR_LOCAL_SUFFIX), array('disable' => 'pantheon_apachesolr', 'ssh-options' => $site['ssh-options'], 'y', 'progress'));
    }
    else {
      drush_log(dt("You need to have sql-sync-pipe installed to use this command."), 'error');
      return;
    }
  }

  // Download and import the latest archive
  if (!$site['terminatur']['database']['download-data'] = _terminatur_get_download_link($site, 'database')) {
    return;
  }

  // Parse download data and begin the download
  $result = $site['terminatur']['database']['download-data'];
  $data = json_decode($result['json']);
  $filename = strstr(basename($data->url), '?', '_');
  $dir = strstr($filename, '.', true);
  $cache_duration = 86400*365;
  $path = _drush_download_file($data->url, TERMINATUR_TMP_DIR . $filename, $cache_duration);
  if (!$path && !drush_get_context('DRUSH_SIMULATE')) {
    drush_log(dt("Download failed. You may want to run 'drush ta' to refresh your download links."), 'error');
    return;
  }
  if (!drush_shell_exec("gunzip " . $path . " -f && " . $databases['default']['default']['connect'] . " < " . str_replace('sql.gz', 'sql', $path))) {
    drush_log(dt("Import failed. Make sure you have gunzip installed."), 'error');
  }
}

/**
 * Downloads a dump of the Pantheon database
 */
function _terminatur_data_download($site, $destination, $db_user, $db_pass, $db_host, $db_port, $pipe = FALSE) {
  // Get the download link data
  if (!$site['terminatur']['database']['download-data'] = _terminatur_get_download_link($site, 'database')) {
    return;
  }
  // Parse download data and begin the download
  $result = $site['terminatur']['database']['download-data'];
  $data = json_decode($result['json']);
  $filename = strstr(basename($data->url), '?', '_');
  $dir = strstr($filename, '.', true);
  $cache_duration = 86400*365;
  $path = _drush_download_file($data->url, getenv('HOME') .  DIRECTORY_SEPARATOR . $filename, $cache_duration);
  if (!$path && !drush_get_context('DRUSH_SIMULATE')) {
    drush_log(dt("Download failed."), 'error');
  }
}

/**
 * Gets a valid local MySQL connection string
 */
function _terminatur_data_get_db_creds($site, $destination, $db_user, $db_pass, $db_host, $db_port) {
  $db_url = 'mysql://' . $db_user . ':' . $db_pass . '@' . $db_host . ':' . $db_port . '/' . $site['terminatur']['database']['local-db'];
  // Check to see if code exists
  if (is_dir($destination . $site['machine-name'])) {
    // Validate settings file if applicable
    if (file_exists($destination . $site['machine-name'] . "/sites/default/settings.php")) {
      $settings_file = $destination . $site['machine-name'] . "/sites/default/settings.php";
      if (!$databases = _terminatur_settings_validate($settings_file)) {
        $databases = array();
        $databases = _terminatur_data_parse_db_url($db_url, NULL);
        if (!$databases = _terminatur_settings_build($settings_file, $databases)) {
          return FALSE;
        }
      }
      // This means we should have a legit settings file with legit db creds
      // Let's build a local alias for it
      _terminatur_aliases_add_alias($site, $destination);
      return $databases;
    }
  }
  else {
    return _terminatur_data_test_db_connection(_terminatur_data_parse_db_url($db_url, NULL));
  }
}

/**
 * Parse a D6 db_url into a D7 array
 *
 * This is basically update_parse_db_url from
 * https://api.drupal.org/api/drupal/includes!update.inc/function/update_parse_db_url/7
 *
 */
function _terminatur_data_parse_db_url($db_url, $db_prefix) {
  $databases = array();
  if (!is_array($db_url)) {
    $db_url = array('default' => $db_url);
  }
  foreach ($db_url as $database => $url) {
    $url = parse_url($url);
    $databases[$database]['default'] = array(
      // MySQLi uses the mysql driver.
      'driver' => $url['scheme'] == 'mysqli' ? 'mysql' : $url['scheme'],
      // Remove the leading slash to get the database name.
      'database' => substr(urldecode($url['path']), 1),
      'username' => urldecode($url['user']),
      'password' => isset($url['pass']) ? urldecode($url['pass']) : '',
      'host' => urldecode($url['host']),
      'port' => isset($url['port']) ? urldecode($url['port']) : '',
    );
    if (isset($db_prefix)) {
      $databases[$database]['default']['prefix'] = $db_prefix;
    }
  }
  return $databases;
}

/**
 * Parse a D6 db_url into a D7 array
 */
function _terminatur_data_test_db_connection(&$databases) {
  // Make sure the database actually exists
  $mysqli = new mysqli($databases['default']['default']['host'], $databases['default']['default']['username'], $databases['default']['default']['password'], NULL, (int) $databases['default']['default']['port']);
  if ($mysqli->connect_error) {
    return FALSE;
  }
  if (!$mysqli->query('CREATE DATABASE IF NOT EXISTS ' . $databases['default']['default']['database'])) {
    return FALSE;
  }
  $mysqli->close();
  // Include a connection string
  $databases['default']['default']['connect'] = "mysql -u" . $databases['default']['default']['username'] . " -p" . $databases['default']['default']['password'] . " -h" . $databases['default']['default']['host'] . " " . $databases['default']['default']['database'];
  return $databases;
}
