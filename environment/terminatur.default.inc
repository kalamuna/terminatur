<?php

 /**
 * @file
 *  default.inc: Default settings
 */

// Constants
define('TERMINATUR_LOCAL_SUFFIX', '.local');
define('TERMINATUR_DRUSH_DIR', getenv('HOME') . DIRECTORY_SEPARATOR . '.drush' . DIRECTORY_SEPARATOR);

define('TERMINATUR_DEFAULT_WEBROOT', '/var/www/');
define('TERMINATUR_DEFAULT_DB_USER', 'root');
define('TERMINATUR_DEFAULT_DB_PASS', 'password');
define('TERMINATUR_DEFAULT_DB_HOST', 'localhost');
define('TERMINATUR_DEFAULT_DB_PORT', '3306');

define('TERMINATUR_DEFAULT_DEFAULT_DIR', '/sites/default/');
define('TERMINATUR_DEFAULT_FILES_DIR', 'files');
