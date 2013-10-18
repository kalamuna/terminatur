<?php

 /**
 * @file
 *  kalastack.inc: Functions to integration with Kalastack
 */

// Constants
define('TERMINATUR_LOCAL_SUFFIX', '.kala');
define('TERMINATUR_LOCAL_PREFIX', 'kalastack');
define('TERMINATUR_DB_SUFFIX', '_kala');
define('TERMINATUR_DRUSH_DIR', '/etc/drush/');
define('TERMINATUR_LOCAL_ALIASES_FILE', TERMINATUR_DRUSH_DIR . TERMINATUR_LOCAL_PREFIX . '.aliases.drushrc.php');

define('TERMINATUR_DEFAULT_WEBROOT', '/var/www/');
define('TERMINATUR_DEFAULT_DB_USER', 'root');
define('TERMINATUR_DEFAULT_DB_PASS', 'password');
define('TERMINATUR_DEFAULT_DB_HOST', 'localhost');
define('TERMINATUR_DEFAULT_DB_PORT', '3306');

define('TERMINATUR_DEFAULT_DEFAULT_DIR', '/sites/default/');
define('TERMINATUR_DEFAULT_FILES_DIR', 'files');
define('TERMINATUR_SETTINGS_CONDITIONAL', "if (isset(\$_SERVER['KALABOX']) &&  \$_SERVER['KALABOX'] === 'on') { \n");

define('TERMINATUR_VHOST_AVAIL_DIR', '/etc/kalastack/vhosts/sites-available/');
define('TERMINATUR_VHOST_ENABLED_DIR', '/etc/kalastack/vhosts/sites-enabled/');
define('TERMINATUR_VHOST_RESTART', 'sudo service nginx restart');

define('TERMINATUR_HOSTS_FILE', '/etc/hosts');

define('TERMINATUR_DEFAULT_PROFILE', 'panopoly');
define('TERMINATUR_DEFAULT_SITENAME', 'My Awesome Site');
define('TERMINATUR_DEFAULT_USERPASS', 'admin');

// Set our own log printing function:
drush_set_context('DRUSH_LOG_CALLBACK', 'terminatur_print_log');

/**
 * Creates a vhost for a newly built or created site
 */
function _terminatur_vhost_add_kalastack($machine_name, $destination) {
  // Get the site machine name and parse it correctly
  // Create a vhost if it doesnt already exit
  if (!file_exists(TERMINATUR_VHOST_AVAIL_DIR . $machine_name)) {
    // Make sure a site is actually where it is supposed to be
    if (is_dir($destination . $machine_name)) {
      // Grab a template
      copy("/etc/nginx/sites-available/drupaldefault", TERMINATUR_TMP_DIR . $machine_name);
      $vhost_default_string = file_get_contents(TERMINATUR_TMP_DIR . $machine_name);

      // Load up default drupal vhost and update with new site info
      $search = array("drupal.kala", "/var/www/drupal", "# THIS IS MEANT TO BE USED FOR BUILDING OTHER DRUPAL VHOSTS");
      $replace = array($machine_name . TERMINATUR_LOCAL_SUFFIX, $destination . $machine_name, "");
      $vhost_default_string = str_replace($search, $replace, $vhost_default_string);

      // Write the file and symlink it to sites-enabled
      file_put_contents(TERMINATUR_TMP_DIR . $machine_name, $vhost_default_string);
      copy(TERMINATUR_TMP_DIR . $machine_name, TERMINATUR_VHOST_AVAIL_DIR . $machine_name);
      symlink(TERMINATUR_VHOST_AVAIL_DIR . $machine_name, TERMINATUR_VHOST_ENABLED_DIR . $machine_name);

      // Restart nginx so the site actually is active
      _terminatur_vhost_restart_kalastack();
    }
  }
}

/**
 * Removes a vhost for a newly built or created site
 */
function _terminatur_vhost_remove_kalastack($machine_name) {
  // Do it this way so we can use SUDO and be sure
  // Also only do this if we have a machine_name otherwise we will kill all our
  // vhosts and that is bad
  if (isset($machine_name)) {
    drush_shell_exec("sudo rm -rf " . TERMINATUR_VHOST_AVAIL_DIR . $machine_name);
    drush_shell_exec("sudo rm -rf " . TERMINATUR_VHOST_ENABLED_DIR . $machine_name);

    // Restart nginx so the site actually is active
    _terminatur_vhost_restart_kalastack();
  }
}

/**
 * Restart the webserver
 */
function _terminatur_vhost_restart_kalastack() {
  drush_shell_exec(TERMINATUR_VHOST_RESTART);
}

/**
 * Creates an entry for the hosts file
 */
function _terminatur_host_add_kalastack($hostname) {
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
function _terminatur_host_remove_kalastack($hostname) {
  // @todo make this less ghetto
  // Actually check for hostname otherwise it wipes out other important stuff!
  if (isset($hostname)) {
    drush_shell_exec("sed /" . $hostname . "/d " . TERMINATUR_HOSTS_FILE . " > " . TERMINATUR_TMP_DIR . "hosts");
    drush_shell_exec("sudo cp -f /tmp/hosts /etc/hosts");
    unlink(TERMINATUR_TMP_DIR . "hosts");
  }
}

/**
 * Additional tasks to run on pullsite
 */
function _terminatur_pullsite_kalastack($site) {
  // Grab thumbnail image if on Kalabox
  if (isset($site['thumbnail'])) {
    // Check for and create an images directory if applicable
    if (!is_dir(TERMINATUR_DEFAULT_WEBROOT . '.images')) {
      mkdir(TERMINATUR_DEFAULT_WEBROOT . '.images');
    }

    // Download the thumbnail
    _terminatur_download_image_kalastack($site['thumbnail'], TERMINATUR_DEFAULT_WEBROOT . '.images/' . $site['machine-name'] . '.png');
  }
}

/**
 * Build a local alias
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 */
function _terminatur_alias_add_kalastack($site, $destination) {
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
    // If built from somewhere, indicate that here
    if (isset($site['built-from'])) {
      $aliases[$site['machine-name'] . TERMINATUR_LOCAL_SUFFIX]['built-from'] = $site['built-from'];
    }
    // if has Thumbnail
    // @todo: Want to make this kalastack specific at some point
    if (isset($site['thumbnail'])) {
      $aliases[$site['machine-name'] . TERMINATUR_LOCAL_SUFFIX]['thumbnail'] = 'http://root.kala/.images/' . $site['machine-name'] . '.png';
    }
  }

  // Sort so that last added is listed first
  $aliases = array_reverse($aliases);
  // Write the file
  _terminatur_alias_write_aliases_kalastack($aliases);
}

/**
 * Remove a local alias
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 */
function _terminatur_alias_remove_kalastack($machine_name) {
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
  _terminatur_alias_write_aliases_kalastack($aliases);
}

/**
 * Write the new vhost file
 */
function _terminatur_alias_write_aliases_kalastack($aliases) {
  // Build output
  // @todo use other output functions
  $output = "<?php \n\n";
  foreach ($aliases as $key => $value) {
    $output .= "\$aliases['" . $key . "'] = array(\n";
    $output .= "  'root' => '" . $value['root'] . "',\n";
    $output .= "  'uri' => '" . $value['uri'] . "',\n";
    $output .= "  'read-name' => '" . addslashes($value['read-name']) .  "',\n";
    $output .= "  'machine-name' => '" . $value['machine-name'] .  "',\n";
    // Only include built froms if they exist
    if (isset($value['built-from'])) {
      $output .= "  'built-from' => '" . $value['built-from'] .  "',\n";
    }
    // if has Thumbnail
    // @todo: Want to make this kalastack specific at some point
    if (isset($value['thumbnail'])) {
      $output .= "  'thumbnail' => '" . $value['thumbnail'] .  "',\n";
    }
    $output .= ");\n";
  }

  // Write file
  file_put_contents(TERMINATUR_LOCAL_ALIASES_FILE, $output);
}

/**
 * Override of _drush_print_log().
 * It's mostly the same except for having non-error mesages print to standard
 * out instead of standard error.
 */
function terminatur_print_log($entry) {
  if (drush_get_context('DRUSH_NOCOLOR')) {
    $red = "[%s]";
    $yellow = "[%s]";
    $green = "[%s]";
  }
  else {
    $red = "\033[31;40m\033[1m[%s]\033[0m";
    $yellow = "\033[1;33;40m\033[1m[%s]\033[0m";
    $green = "\033[1;32;40m\033[1m[%s]\033[0m";
  }

  $verbose = drush_get_context('DRUSH_VERBOSE');
  $debug = drush_get_context('DRUSH_DEBUG');

  $return = TRUE;
  $output_type = NULL; // Will default to standard out.
  switch ($entry['type']) {
    case 'warning':
    case 'cancel':
      $type_msg = sprintf($yellow, $entry['type']);
      break;
    case 'failed':
    case 'error':
      $type_msg = sprintf($red, $entry['type']);
      $return = FALSE;
      $output_type = STDERR;
      break;
    case 'ok':
    case 'completed':
    case 'success':
    case 'status':
      // In quiet mode, suppress progress messages
      if (drush_get_context('DRUSH_QUIET')) {
        return TRUE;
      }
      $type_msg = sprintf($green, $entry['type']);
      break;
    case 'notice':
    case 'message':
    case 'info':
      if (!$verbose) {
        // print nothing. exit cleanly.
        return TRUE;
      }
      $type_msg = sprintf("[%s]", $entry['type']);
      break;
    default :
      if (!$debug) {
        // print nothing. exit cleanly.
        return TRUE;
      }
      $type_msg = sprintf("[%s]", $entry['type']);
      break;
  }

  // When running in backend mode, log messages are not displayed, as they will
  // be returned in the JSON encoded associative array.
  if (drush_get_context('DRUSH_BACKEND')) {
    return $return;
  }

  $columns = drush_get_context('DRUSH_COLUMNS', 80);

  $width[1] = 11;
  // Append timer and memory values.
  if ($debug) {
    $timer = sprintf('[%s sec, %s]', round($entry['timestamp'] -DRUSH_REQUEST_TIME, 2), drush_format_size($entry['memory']));
    $entry['message'] = $entry['message'] . ' ' . $timer;
  }

  $width[0] = ($columns - 11);

  $format = sprintf("%%-%ds%%%ds", $width[0], $width[1]);

  // Place the status message right aligned with the top line of the error message.
  $message = wordwrap($entry['message'], $width[0]);
  $lines = explode("\n", $message);
  $lines[0] = sprintf($format, $lines[0], $type_msg);
  $message = implode("\n", $lines);
  drush_print($message, 0, $output_type);
  return $return;
}

/*
 * Grabs a Pantheon thumbnail and saves it if on Kalabox
 */
function _terminatur_download_image_kalastack($url, $destination) {
  $ch = curl_init ($url);
  curl_setopt($ch, CURLOPT_HEADER, 0);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_BINARYTRANSFER, 1);

  $raw = curl_exec($ch);
  curl_close ($ch);
  // Let's get up to date thumbs!
  if (file_exists($destination)) {
    unlink($destination);
  }

  $fp = fopen($destination, 'x');
  fwrite($fp, $raw);
  fclose($fp);
}

