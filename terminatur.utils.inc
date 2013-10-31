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
    return drush_set_error('TERMINATUR_ERROR', 'Not a valid Pantheon site.');
  }
  if ($site) {
    return $site;
  }
  if ($allow_local) {
    return _terminatur_aliases_get_local_site($sitename);
  }
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
    else {
      return drush_set_error('TERMINATUR_ERROR', 'Not a valid local site.');
    }
  }
  return FALSE;
}
