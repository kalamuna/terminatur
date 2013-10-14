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
define('TERMINATUR_LOCAL_ALIASES_FILE', TERMINATUR_DRUSH_DIR . TERMINATUR_LOCAL_PREFIX . '.aliases.drushrc.php');

define('TERMINATUR_DEFAULT_WEBROOT', '/var/www/');
define('TERMINATUR_DEFAULT_DB_USER', 'root');
define('TERMINATUR_DEFAULT_DB_PASS', 'password');
define('TERMINATUR_DEFAULT_DB_HOST', 'localhost');
define('TERMINATUR_DEFAULT_DB_PORT', '3306');

define('TERMINATUR_DEFAULT_DEFAULT_DIR', '/sites/default/');
define('TERMINATUR_DEFAULT_FILES_DIR', 'files');
define('TERMINATUR_SETTINGS_CONDITIONAL', "if (!isset(\$_SERVER['PANTHEON_ENVIRONMENT'])) { \n");

define('TERMINATUR_VHOST_AVAIL_DIR', 'SOMETHING?');
define('TERMINATUR_VHOST_ENABLED_DIR', 'SOMETHING ELSE?');
define('TERMINATUR_VHOST_RESTART', 'sudo service nginx restart');

define('TERMINATUR_HOSTS_FILE', '/etc/hosts');

define('TERMINATUR_DEFAULT_PROFILE', 'drupal7');
define('TERMINATUR_DEFAULT_SITENAME', 'My Awesome Site');
define('TERMINATUR_DEFAULT_USERPASS', 'admin');

/**
 * Creates a vhost for a newly built or created site
 */
function _terminatur_vhost_add_default($machine_name, $destination)  {
  //@todo: need some sort of default implementation here
}

/**
 * Removes a vhost for a newly built or created site
 */
function _terminatur_vhost_remove_default($machine_name) {
  //@todo: need some sort of default implementation here
}

/**
 * Restarts the webserver
 */
function _terminatur_vhost_restart_default() {
  drush_shell_exec(TERMINATUR_VHOST_RESTART);
}

/**
 * Creates an entry for the hosts file
 */
function _terminatur_host_add_default($hostname) {
  //@todo: need some sort of default implementation here
}

/**
 * Remove hostname from hosts file
 */
function _terminatur_host_remove_default($hostname) {
  //@todo: need some sort of default implementation here
  }
