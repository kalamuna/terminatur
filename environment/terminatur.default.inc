<?php

 /**
 * @file
 *  default.inc: Default settings
 */

// Constants
define('TERMINATUR_LOCAL_SUFFIX', '.local');
define('TERMINATUR_LOCAL_PREFIX', 'local');
define('TERMINATUR_DB_SUFFIX', '_local');
define('TERMINATUR_DRUSH_DIR', getenv('HOME') . DIRECTORY_SEPARATOR . '.drush' . DIRECTORY_SEPARATOR);
define('TERMINATUR_LOCAL_ALIASES_FILE', TERMINATUR_DRUSH_DIR . TERMINATUR_LOCAL_PREFIX . '.aliases.drushrc.php');

define('TERMINATUR_DEFAULT_WEBROOT', '/var/www/');
define('TERMINATUR_DEFAULT_DB_USER', 'root');
define('TERMINATUR_DEFAULT_DB_PASS', 'password');
define('TERMINATUR_DEFAULT_DB_HOST', 'localhost');
define('TERMINATUR_DEFAULT_DB_PORT', '3306');

define('TERMINATUR_DEFAULT_DEFAULT_DIR', '/sites/default/');
define('TERMINATUR_DEFAULT_FILES_DIR', 'files');
define('TERMINATUR_DEFAULT_FILES_PRIVATE_DIR', 'files/private');
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
}

/**
 * Creates an entry for the hosts file
 */
function _terminatur_host_add_default($hostname) {
  $hosts_file_str = file_get_contents(TERMINATUR_HOSTS_FILE);
  $new_host = "127.0.0.1\t$hostname\n";
  if (!strstr($hosts_file_str, $new_host)) {
    $new_hosts_file_str =  $new_host . $hosts_file_str;
    file_put_contents(TERMINATUR_TMP_DIR . "hosts", $new_hosts_file_str);
    // Doing it this way because we need sudo
    drush_shell_exec("sudo cp -f " . TERMINATUR_TMP_DIR . "hosts " . TERMINATUR_HOSTS_FILE);
    unlink(TERMINATUR_TMP_DIR . "hosts");
  }
}

/**
 * Remove hostname from hosts file
 */
function _terminatur_host_remove_default($hostname) {
  // @todo make this less ghetto
  // Actually check for hostname otherwise it wipes out other important stuff!
  if (isset($hostname)) {
    drush_shell_exec("sed /" . $hostname . "/d " . TERMINATUR_HOSTS_FILE . " > " . TERMINATUR_TMP_DIR . "hosts");
    drush_shell_exec("sudo cp -f /tmp/hosts /etc/hosts");
    unlink(TERMINATUR_TMP_DIR . "hosts");
  }
}

/**
 * Adds a local new alias entrys
 */
function _terminatur_alias_add_default($site, $destination) {
    // Create an aliases file if there are no local sites
  $aliases = array();
  if (!file_exists(TERMINATUR_LOCAL_ALIASES_FILE)) {
    file_put_contents(TERMINATUR_LOCAL_ALIASES_FILE, "");
  }

  // Load the local aliases file
  require TERMINATUR_LOCAL_ALIASES_FILE;
  // Restore to first made first listed
  $aliases = array_reverse($aliases);
  if (!isset($aliases[$site['machine-name'] . TERMINATUR_LOCAL_SUFFIX])) {
    // Build the local alias
    $aliases[$site['machine-name'] . TERMINATUR_LOCAL_SUFFIX] = array(
      'root' => $destination . $site['machine-name'],
      'uri' => $site['machine-name'] . TERMINATUR_LOCAL_SUFFIX,
      'read-name' => $site['machine-name'],
      'machine-name' => $site['machine-name'],
    );
  }

  // Sort so that last added is listed first
  $aliases = array_reverse($aliases);
  // Write the file
  _terminatur_alias_write_aliases_default($aliases);
}

/**
 * Remove local alias from local aliases file
 */
function _terminatur_alias_remove_default($machine_name) {
  // Load alias file if it exists
  if (file_exists(TERMINATUR_LOCAL_ALIASES_FILE)) {
    $aliases = array();
    require TERMINATUR_LOCAL_ALIASES_FILE;
  }
  else {
    return;
  }

  // Load the kala aliases file
  if (isset($aliases[$machine_name . TERMINATUR_LOCAL_SUFFIX])) {
    unset($aliases[$machine_name . TERMINATUR_LOCAL_SUFFIX]);
  }

  // Sort so that last added is listed first
  $aliases = array_reverse($aliases);
  // Write the file
  _terminatur_alias_write_aliases_default($aliases);
}

/**
 * Write the new alias file
 */
function _terminatur_alias_write_aliases_default($aliases) {  // Build output
  // @todo use other output functions
  $output = "<?php \n\n";
  foreach ($aliases as $key => $value) {
    $output .= "\$aliases['" . $key . "'] = array(\n";
    $output .= "  'root' => '" . $value['root'] . "',\n";
    $output .= "  'uri' => '" . $value['uri'] . "',\n";
    $output .= "  'read-name' => '" . addslashes($value['read-name']) .  "',\n";
    $output .= "  'machine-name' => '" . $value['machine-name'] .  "',\n";
    $output .= ");\n";
  }

  // Write file
  file_put_contents(TERMINATUR_LOCAL_ALIASES_FILE, $output);
}
