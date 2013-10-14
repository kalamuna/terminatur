<?php

 /**
 * @file
 *  default.inc: Default settings
 */

// Constants
define('TERMINATUR_LOCAL_SUFFIX', '.local');
define('TERMINATUR_LOCAL_PREFIX', 'local');
define('TERMINATUR_DC_SUFFIX', '_local');
define('TERMINATUR_DRUSH_DIR', getenv('HOME') . DIRECTORY_SEPARATOR . '.drush' . DIRECTORY_SEPARATOR);
define('TERMINATUR_LOCAL_ALIASES_FILE', TERMINATUR_DRUSH_DIR . 'local.aliases.drushrc.php');

define('TERMINATUR_DEFAULT_WEBROOT', '/var/www/');
define('TERMINATUR_DEFAULT_DB_USER', 'root');
define('TERMINATUR_DEFAULT_DB_PASS', 'password');
define('TERMINATUR_DEFAULT_DB_HOST', 'localhost');
define('TERMINATUR_DEFAULT_DB_PORT', '3306');

define('TERMINATUR_DEFAULT_DEFAULT_DIR', '/sites/default/');
define('TERMINATUR_DEFAULT_FILES_DIR', 'files');

define('TERMINATUR_VHOST_AVAIL_DIR', 'SOMETHING?');
define('TERMINATUR_VHOST_AVAIL_DIR', 'SOMETHING ELSE?');

/**
 * Creates a vhost for a newly built or created site
 */
function _terminatur_vhost_add_default($site, $destination) {
  //@todo: need some sort of default implementation here
}

/**
 * Removes a vhost for a newly built or created site
 */
function _terminatur_vhost_remove_default($site, $destination) {
  //@todo: need some sort of default implementation here
}
