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
    return FALSE;
  }

  // Try to get the most up to date data with sql-sync-pipe
  if ($pipe) {
    return _terminatur_data_pipe($site);
  }

  // Parse download data and begin the download
  if (!$path = _terminatur_data_download($site)) {
    return _terminatur_data_pipe($site);
  }
  else {
  // Import downloaded data
    if (!drush_shell_exec("gunzip " . $path . " -f && " . $databases['default']['default']['connect'] . " < " . str_replace('sql.gz', 'sql', $path))) {
      drush_log(dt("Couldn't import downloaded backup. Trying pipe instead."), 'warning');
      return _terminatur_data_pipe($site);
    }
  }
  return TRUE;
}

/**
 * Downloads a dump of the Pantheon database
 */
function _terminatur_data_download($site) {
  // Get the download link data
  if (!$site['terminatur']['database']['download-data'] = _terminatur_get_download_link($site, 'database')) {
    drush_log(dt("You have no backups. Trying pipe instead."), 'warning');
    return FALSE;
  }
  // Parse download data and begin the download
  $result = $site['terminatur']['database']['download-data'];
  $data = json_decode($result['json']);
  $filename = strstr(basename($data->url), '?', '_');
  $dir = strstr($filename, '.', true);
  $cache_duration = 86400*365;
  $path = _drush_download_file($data->url, getenv('HOME') .  DIRECTORY_SEPARATOR . $filename, $cache_duration);
  if (!$path && !drush_get_context('DRUSH_SIMULATE')) {
    drush_log(dt("Couldn't download backup. Trying pipe instead."), 'warning');
    return FALSE;
  }
  return $path;
}

/**
 * Downloads a dump of the Pantheon database
 */
function _terminatur_data_pipe($site) {
  if (function_exists('drush_sql_sync_pipe')) {
    if (!$local_site = _terminatur_aliases_get_local_site($site['machine-name'] . TERMINATUR_LOCAL_SUFFIX)) {
      drush_log(dt("You need to have a local site set up to use pipe."), 'error');
      return FALSE;
    }
    // Unreap my heart: make sure DB server is awake
    if (!drush_invoke_process("@pantheon." . $site['machine-name'] . "." . $site['env'], 'core-status', array(), array())) {
      drush_log(dt("Cannot connect to Pantheon."), 'error');
      return FALSE;
    }
    // Force it
    drush_set_context('DRUSH_AFFIRMATIVE', TRUE);
    drush_set_option('ssh-options', $site['ssh-options']);
    drush_set_option('disable', 'pantheon_apachesolr');
    drush_sql_sync_pipe("@pantheon." . $site['machine-name'] . "." . $site['env'], "@" . TERMINATUR_LOCAL_PREFIX . "." . $site['machine-name'] . TERMINATUR_LOCAL_SUFFIX);
    return TRUE;
  }
  else {
    drush_log(dt("You need to have sql-sync-pipe installed to use this command."), 'error');
    return FALSE;
  }
}

/**
 * Gets a valid local MySQL connection string
 */
function _terminatur_data_get_db_creds($site, $destination, $db_user, $db_pass, $db_host, $db_port) {
  $db_url = 'mysql://' . $db_user . ':' . $db_pass . '@' . $db_host . ':' . $db_port . '/' . $site['machine-name'] . TERMINATUR_DB_SUFFIX;
  // Check to see if code exists
  if (is_dir($destination . $site['machine-name'])) {
    // Validate settings file if applicable
    if (file_exists($destination . $site['machine-name'] . TERMINATUR_DEFAULT_DEFAULT_DIR . "settings.php")) {
      $settings_file = $destination . $site['machine-name'] . TERMINATUR_DEFAULT_DEFAULT_DIR . "settings.php";
      if (!$databases = _terminatur_settings_validate($settings_file)) {
        $databases = array();
        $databases = _terminatur_data_parse_db_url($db_url, NULL);
        if (!$databases = _terminatur_settings_build($settings_file, $databases)) {
          return FALSE;
        }
      }
      // This means we should have a legit settings file with legit db creds
      // Let's build a local alias for it
      // Build alias callback function
      $get_alias_func = '_terminatur_alias_add_' . TERMINATUR_ENV;
      $get_alias_func($site, $destination);
      return $databases;
    }
    elseif (file_exists($destination . $site['machine-name'] . TERMINATUR_DEFAULT_DEFAULT_DIR . "default.settings.php")) {
      // Sometimes settings.php is gitignored so we should instantiate one and try again
      copy($destination . $site['machine-name'] . TERMINATUR_DEFAULT_DEFAULT_DIR . "default.settings.php", $destination . $site['machine-name'] . TERMINATUR_DEFAULT_DEFAULT_DIR . "settings.php");
      _terminatur_data_get_db_creds($site, $destination, $db_user, $db_pass, $db_host, $db_port);
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
      'database' => str_replace("-", "_", substr(urldecode($url['path']), 1)),
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
    drush_log(dt("Could not connect to MySQL database. Verify your credentials and try again."), 'error');
    return FALSE;
  }
  if (!$mysqli->query('CREATE DATABASE IF NOT EXISTS ' . $databases['default']['default']['database'])) {
    drush_log(dt("Could not create new database. Verify you have permission and try again"), 'error');
    return FALSE;
  }
  $mysqli->close();
  // Include a connection string
  $databases['default']['default']['connect'] = "mysql -u" . $databases['default']['default']['username'] . " -p" . $databases['default']['default']['password'] . " -h" . $databases['default']['default']['host'] . " " . $databases['default']['default']['database'];
  return $databases;
}

/**
 * Removes the database
 */
function _terminatur_data_remove($site, $destination, $db_user, $db_pass, $db_host, $db_port) {
  $databases = array();
  if (!$databases = _terminatur_data_get_db_creds($site, $destination, $db_user, $db_pass, $db_host, $db_port)) {
    drush_log(dt("You have entered incorrect MySQL credentials."), 'error');
    return;
  }
  $mysqli = new mysqli($databases['default']['default']['host'], $databases['default']['default']['username'], $databases['default']['default']['password'], NULL, (int) $databases['default']['default']['port']);
  $mysqli->query('DROP DATABASE ' . $databases['default']['default']['database']);
  $mysqli->close();
}
