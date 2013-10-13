<?php

 /**
 * @file
 *  code.inc: Helper functions to grab code
 */

/**
 * Gets the codebase for the Pantheon site with git
 */
function _terminatur_code_git($site, $destination) {
  // Check if the codebase already exists
  if (file_exists($destination . $site['machine-name'] . "/.git/config")) {
    drush_shell_exec("cat " . $destination . $site['machine-name'] . "/.git/config | grep url > " . TERMINATUR_TMP_DIR . "gitconfig" . $site['machine-name']);
    $gitconfig_string = trim(file_get_contents(TERMINATUR_TMP_DIR . "gitconfig" . $site['machine-name']));
    drush_shell_exec("rm /tmp/gitconfig" . $site['machine-name']);

    // Codebase does exist, just refresh that code
    if ($gitconfig_string == "url = " . $site['terminatur']['code']['git-connect']) {
      drush_shell_exec("git --git-dir=" . $destination . $site['machine-name'] . "/.git pull");
      return;
    }
    else {
      drush_shell_exec("rm -rf " . $destination . $site['machine-name']);
    }
  }
  // Git clone for the first time
  drush_shell_exec("mkdir " . $destination . $site['machine-name'] . " && cd " . $destination . " && git clone " . $site['terminatur']['code']['git-connect'] . " " . $site['machine-name']);
}

/**
 * Gets the codebase for the Pantheon site via S3 download
 */
function _terminatur_code_download($site, $destination) {
  // Get the download link data
  if (!$site['terminatur']['code']['download-data'] = _terminatur_get_download_link($site, 'code')) {
    return;
  }
  // Parse download data and begin the download
  $result = $site['terminatur']['code']['download-data'];
  $data = json_decode($result['json']);
  $filename = strstr(basename($data->url), '?', '_');
  $dir = strstr($filename, '.', true);
  $cache_duration = 86400*365;
  $path = _drush_download_file($data->url, TERMINATUR_TMP_DIR . $filename, $cache_duration);
  if ($path || drush_get_context('DRUSH_SIMULATE')) {
    drush_tarball_extract($path, TERMINATUR_TMP_DIR);
    drush_shell_exec("rm -rf " . $destination . $site['machine-name']);
    drush_shell_exec("mv " . TERMINATUR_TMP_DIR . $dir . " " . $destination . $site['machine-name']);
  }
  else {
    drush_log(dt("Download failed."), 'error');
  }
}
