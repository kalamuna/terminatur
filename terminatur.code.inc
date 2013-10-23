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
    // @todo get this out of drush_shell_exec and into PHP
    drush_shell_exec("cat " . $destination . $site['machine-name'] . "/.git/config | grep url > " . TERMINATUR_TMP_DIR . "gitconfig" . $site['machine-name']);
    $gitconfig_string = trim(file_get_contents(TERMINATUR_TMP_DIR . "gitconfig" . $site['machine-name']));
    drush_shell_exec("rm /tmp/gitconfig" . $site['machine-name']);

    // Codebase does exist, just refresh that code
    if ($gitconfig_string == "url = " . $site['terminatur']['code']['git-connect']) {
      // We need to use this environmental variable bullshit to ensure we actually get relevant output
      if (!drush_shell_exec("GIT_SSH='" . $site['terminatur']['code']['git-ssh'] . "' && git --git-dir=" . $destination . $site['machine-name'] . "/.git pull")) {
        drush_log(dt("Could not git pull your code. Check that your SSH key is loaded."), 'error');
        return FALSE;
      }
      return TRUE;
    }
    else {
      // Do some checking so we don't delete the whole webroot!
      if (isset($site['machine-name'])) {
        if (!drush_shell_exec("rm -rf " . $destination . $site['machine-name'])) {
          drush_log(dt("Could not remove the existing codebase. Please remove manually and then try again."), 'error');
          return FALSE;
        }
      }
    }
  }
  // Git clone for the first time
  mkdir($destination . $site['machine-name'], 0777, TRUE);
  // We need to use this environmental variable bullshit to ensure we actually get relevant output
  if (!drush_shell_exec("GIT_SSH='" . $site['terminatur']['code']['git-ssh'] . "' cd " . $destination . " && git clone " . $site['terminatur']['code']['git-connect'] . " " . $site['machine-name'])) {
    drush_log(dt("Could not clone code from the remote repository. Check that your SSH key is loaded."), 'error');
    return FALSE;
  }
  return TRUE;
}

/**
 * Gets the codebase for the Pantheon site via S3 download
 */
function _terminatur_code_download($site, $destination) {
  // Get the download link data
  if (!$site['terminatur']['code']['download-data'] = _terminatur_get_download_link($site, 'code')) {
    return FALSE;
  }
  // Parse download data and begin the download
  $result = $site['terminatur']['code']['download-data'];
  $data = json_decode($result['json']);
  $filename = strstr(basename($data->url), '?', '_');
  $dir = strstr($filename, '.', true);
  $cache_duration = 86400*365;
  $path = _drush_download_file($data->url, TERMINATUR_TMP_DIR . $filename, $cache_duration);
  if ($path || drush_get_context('DRUSH_SIMULATE')) {
    drush_tarball_extract($path);
    drush_shell_exec("rm -rf " . $destination . $site['machine-name']);
    rename(TERMINATUR_TMP_DIR . $dir, $destination . $site['machine-name']);
    return TRUE;
  }
  else {
    drush_log(dt("Download failed."), 'error');
    return FALSE;
  }
}

/**
 * Removes the code
 */
function _terminatur_code_remove($site, $destination) {
  // Remove Code but first make sure we have a machine name before we
  // inadvertently destroy the entire webroot
  if (isset($site['machine-name'])) {
    $web_root = $destination . $site['machine-name'];
    // Make write-protected directory and files writable so we can remove them.
    if (is_dir($web_root . TERMINATUR_DEFAULT_DEFAULT_DIR)) {
      chmod($web_root . TERMINATUR_DEFAULT_DEFAULT_DIR, 0755);
      chmod($web_root . TERMINATUR_DEFAULT_DEFAULT_DIR . 'default.settings.php', 0644);
      chmod($web_root . TERMINATUR_DEFAULT_DEFAULT_DIR . 'settings.php', 0644);
    }
    // Remove the webroot and everything in it.
    // Need to run shell command because PHP isnt great at complete destruction
    drush_shell_exec("rm -rf " . $web_root);
  }
}
