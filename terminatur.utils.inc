<?php

 /**
 * @file
 *  utils.inc: Helper functions for everywhere
 */

/**
 * Determine what environmental file to load
 */
function _terminatur_get_site($sitename, $allow_local = FALSE) {
  $sites = terminatur_bootstrap();
  $site = isset($sites[$sitename]) ? $sites[$sitename] : FALSE;
  if (!$site && !$allow_local) {
    drush_log(dt("Not a valid Pantheon site."), 'error');
    return;
  }
  if ($site) {
    return $site;
  }
  if ($allow_local) {
    return _terminatur_aliases_get_local_site($sitename);
  }
  drush_log(dt("Not a valid a valid Pantheon or local site."), 'error');
}

/**
 * Return an array with local site data
 */
function _terminatur_aliases_get_local_site($sitename) {
  // Load alias file if it exists
  if (file_exists(TERMINATUR_LOCAL_ALIASES_FILE)) {
    $aliases = array();
    require TERMINATUR_LOCAL_ALIASES_FILE;
    if (isset($aliases[$sitename])) {
      return $aliases[$sitename];
    }
  }
  return FALSE;
}
