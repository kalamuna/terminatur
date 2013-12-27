<?php

 /**
 * @file
 *  database.inc: Helper functions to grab data
 */

/**
 * Amount of time to wait for Pantheon to create a database backup.
 */
define('TERMINATUR_DB_BACKUP_TIMEOUT', 900); // 15 minutes.

/**
 * Gets the DB either by downloading a specific backup or creating a new one, then imports into mysql
 *
 * Validates and builds settings files it applicable.
 */
function _terminatur_data_mysql($site, $destination, $db_user, $db_pass, $db_host, $db_port) {
  $databases = array();
  if (!$databases = _terminatur_data_get_db_creds($site, $destination, $db_user, $db_pass, $db_host, $db_port)) {
    return drush_set_error('TERMINATUR_ERROR', 'You have entered incorrect MySQL credentials.');
  }

  // Parse download data and begin the download
  if (!$path = _terminatur_data_download($site)) {
    // If that fails, fall back to creating a new backup.
    $site['terminatur']['database']['download-bucket'] = _terminatur_data_make_backup($site);
    $path = _terminatur_data_download($site);
  }
  else {
  // Import downloaded data
    if (!drush_shell_exec("gunzip " . $path . " -f && " . $databases['default']['default']['connect'] . " < " . str_replace('sql.gz', 'sql', $path))) {
      return drush_set_error('TERMINATUR_ERROR', dt("Couldn't import downloaded backup."));
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
    drush_log(dt("You have no backups. Attempting to create one."), 'warning');
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
    drush_log(dt("Couldn't download backup. Trying to create a new backup instead."), 'warning');
    return FALSE;
  }
  return $path;
}

/**
 * Requests a database backup via Terminus.
 *
 * @param array $site
 *   Site to make the backup for.
 *
 * @return string|bool
 *   New database backup bucket, or FALSE if the call timed out.
 */
function _terminatur_data_make_backup($site) {
  $backup_complete = FALSE;
  $backup_started = time();
  $current_backup = terminus_latest_bucket($site['uuid'], $site['env'], 'database');
  terminus_api_site_make_backup($site['uuid'], $site['env'], 'backup', FALSE, TRUE, FALSE);
  // @todo Check for Terminus API failure.
  // Terminus API just queues the backup, so wait around till it's done.
  while (!$backup_complete) {
    drush_log(dt('Creating database backup...'), 'warning');
    sleep(5);
    // Check if backup finished.
    $latest_backup = terminus_latest_bucket($site['uuid'], $site['env'], 'database');
    if ($latest_backup != $current_backup) {
      $backup_complete = TRUE;
      $current_backup = $latest_backup;
    }
    // Error out if the backup has taken too long.
    elseif ((time() - $backup_started) > TERMINATUR_DB_BACKUP_TIMEOUT) {
      return FALSE;
    }
  }
  drush_log(dt('Database backup complete!'), 'success');
  return $current_backup;
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
    return drush_set_error('TERMINATUR_ERROR', 'Could not connect to MySQL database. Verify your credentials and try again.');
  }
  if (!$mysqli->query('CREATE DATABASE IF NOT EXISTS ' . $databases['default']['default']['database'])) {
    return drush_set_error('TERMINATUR_ERROR', 'Could not create new database. Verify you have permission and try again');
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
    return drush_set_error('TERMINATUR_ERROR', 'You have entered incorrect MySQL credentials.');
  }
  $mysqli = new mysqli($databases['default']['default']['host'], $databases['default']['default']['username'], $databases['default']['default']['password'], NULL, (int) $databases['default']['default']['port']);
  $mysqli->query('DROP DATABASE ' . $databases['default']['default']['database']);
  $mysqli->close();
  return TRUE;
}

/**
 * Gets the database backup bucket for the site.
 *
 * @param array $site
 *   The site we're operating on.
 *
 * @return bool
 *   TRUE on success, FALSE on failure.
 */
function _terminatur_get_data_bucket(&$site) {
  // Authenticate with Terminus.
  _terminatur_get_session();
  // If backup bucket option passed, use it.
  $backup_bucket = drush_get_option('db-backup-bucket');
  if ($backup_bucket && is_string($backup_bucket)) {
    if ($backup_bucket == 'new') {
      $backup_bucket = _terminatur_data_make_backup($site);
    }
    elseif ($backup_bucket == 'latest') {
      $backup_bucket = terminus_latest_bucket($site['uuid'], $site['env'], 'database');
    }
    $site['terminatur']['database']['download-bucket'] = $backup_bucket;
  }
  // If not, ask the user.
  elseif (!_terminatur_choose_data_bucket($site)) {
    return FALSE;
  }
  return TRUE;
}

/**
 * Asks the user what bucket to download the database from.
 *
 * @param array $site
 *   The site we're working with, to which we'll add the chosen download bucket.
 *
 * @return bool
 *   TRUE if user completed input successfully, FALSE if not.
 */
function _terminatur_choose_data_bucket(&$site) {
  // Ask the user if they want to download the latest bucket,
  // create a new one, or select one from a list.
  $bucket_source_options = array(
    'latest' => dt('Get it from the latest Pantheon backup.'),
    'new' => dt('Create a new Pantheon backup and download it.'),
    'pick' => dt('Pick one from a list of Pantheon backups.'),
  );
  $bucket_source_choice = drush_choice($bucket_source_options, dt('How do you want to download your database?'));
  if (!$bucket_source_choice) {
    return drush_set_error('TERMINATUR_ERROR', dt('Command cancelled.'));
  }
  $bucket = NULL;
  // If user opted to make a new backup, carry that out.
  if ($bucket_source_choice == 'new') {
    $bucket = _terminatur_data_make_backup($site);
    if (!$bucket) {
      return drush_set_error('TERMINATUR_ERROR', dt('Failed to create database backup.'));
    }
  }
  // If user opted to use the latest backup, retrieve it.
  elseif ($bucket_source_choice == 'latest') {
    $bucket = terminus_latest_bucket($site['uuid'], $site['env'], 'database');
    if (!$bucket) {
      return drush_set_error('TERMINATUR_ERROR', dt('Unable to retrieve the latest database backup from Pantheon.'));
    }
  }
  // If the user wants to see a list of backups, show it to them and have them select one.
  elseif ($bucket_source_choice == 'pick') {
    $backups = terminus_api_backup_catalog($site['uuid'], $site['env']);
    $backups = isset($backups['json']) ? json_decode($backups['json'], true) : '';
    if (empty($backups)) {
      return drush_set_error('TERMINATUR_ERROR', dt('Sorry, you don\'t have any backups.'));
    }
    $bucket_choices = array();
    foreach($backups as $label => $backup) {
      $parts = explode('_', $label);
      if ($parts[2] != 'database') {
        continue;
      }
      $bucket_choices[$label] = date('F j, Y h:i:s A', $backup['timestamp']);
    }
    ksort($bucket_choices);
    $bucket_choices = array_reverse($bucket_choices);
    $bucket = drush_choice($bucket_choices, dt('Which database backup do you want to download?'));
    if (!$bucket) {
      return drush_set_error('TERMINATUR_ERROR', dt('Command cancelled.'));
    }
    $bucket = str_replace('_database', '', $bucket);
  }
  // Assign the backup bucket to the site.
  $site['terminatur']['database']['download-bucket'] = $bucket;
  return TRUE;
}
