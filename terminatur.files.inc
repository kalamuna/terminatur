<?php

 /**
 * @file
 *  files.inc: Helper functions to download files
 */

/**
 * @file
 *  database.inc: Helper functions to grab data
 */

/**
 * Gets the DB either by pipe or download adn then imports into mysql
 *
 * Validates and builds settings files it applicable.
 */
function _terminatur_files_rysnc($site, $destination) {
  // Build files directories as appropriate
  _terminatur_files_set_up_dirs($site, $destination);


    // RSYNCing files from pantheon
    /*
    drush_shell_exec("rsync -rlztv --ipv4 -e 'ssh " . $site['ssh'] . "' --exclude 'js' --exclude 'css' --exclude 'ctools' --exclude 'imagecache' --exclude 'xmlsitemap' --exclude 'backup_migrate' --exclude 'styles' --exclude 'less' " . $site['env'] . "." . $site['id'] . "@appserver." . $site['env'] . "." . $site['id'] . ".drush.in:files /var/www/" . $site['site'] . "/sites/default/");
    drush_log('Consider your files N*SYNC', 'success');
  }
  else {
    return;
  }
  */
}

/**
 * Downloads a dump of the Pantheon database
 */
function _terminatur_files_download($site, $destination) {
  // Get the download link data
  if (!$site['terminatur']['files']['download-data'] = _terminatur_get_download_link($site, 'files')) {
    return;
  }

  // Build files directories as appropriate
  _terminatur_files_set_up_dirs($site, $destination);

  // Parse download data and begin the download
  $result = $site['terminatur']['files']['download-data'];
  $data = json_decode($result['json']);
  $filename = strstr(basename($data->url), '?', '_');
  $dir = strstr($filename, '.', true);
  $cache_duration = 86400*365;
  $path = _drush_download_file($data->url, TERMINATUR_TMP_DIR . $filename, $cache_duration);
  if ($path || drush_get_context('DRUSH_SIMULATE')) {
    drush_tarball_extract($path);
    drush_shell_exec("rm -rf " . _terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_DIR);
    rename(TERMINATUR_TMP_DIR . "files_" . $site['env'], _terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_DIR);
  }
  else {
    drush_log(dt("Download failed."), 'error');
  }
}

/**
 * Gets the location to download the files
 */
function _terminatur_files_download_location($site, $destination) {
  if (is_dir($destination . $site['machine-name']) && is_writable($destination . $site['machine-name'])) {
    return $destination . $site['machine-name'];
  }
  else {
    return getenv('HOME') . DIRECTORY_SEPARATOR . $site['machine-name'];
  }
}

/**
 * Gets the location to download the files
 */
function _terminatur_files_set_up_dirs($site, $destination) {
  // Check files directory
  if (!is_dir(_terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_DIR)) {
    mkdir(_terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_DIR, 0777, TRUE);
  }
  if (!is_writable(_terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_DIR)) {
    chmod(_terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_DIR, 0777);
  }
}
