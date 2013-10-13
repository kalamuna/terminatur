<?php

 /**
 * @file
 *  kalastack.inc: Functions to integration with Kalastack
 */

/** HOSTS FILE STUFF **/
/**
 * Remove hostname from hosts file
 * @param  [string] $hostname [the hostname to be added]
 */
function terminatur_remove_etc_hosts($hostname) {
  // Remove hosts
  drush_shell_exec("sed /" . $hostname . "/d /etc/hosts > /tmp/hosts");
  drush_shell_exec("sudo cp -f /tmp/hosts /etc/hosts");
  drush_shell_exec("rm /tmp/hosts");
}

/**
 * Update the hosts file
 * @param  [string] $hostname [the hostname to be added]
 */
function terminatur_update_etc_hosts($hostname) {
    // Create new hosts file
    $hosts_file_str = file_get_contents("/etc/hosts");
    $new_host = "127.0.0.1\t$hostname\n";
    if (!strstr($hosts_file_str, $new_host)) {
      $new_hosts_file_str =  $new_host . $hosts_file_str;
      file_put_contents("/tmp/hosts", $new_hosts_file_str);
      drush_shell_exec("sudo cp -f /tmp/hosts /etc/hosts");
      #drush_shell_exec("sudo service nginx restart");
      drush_shell_exec("rm /tmp/hosts");
    }
}

/** VHOST STUFF **/
/**
 * Creates a vhost for a newly built or created site
 */
function terminuatur_vhost() {
  // Get the site machine name and parse it correctly
  $args = drush_get_arguments();
  $siteal = explode('.', $args[1]);
  $sitename = $siteal[0];

  // Create a vhost if it doesnt already exit
  if (!file_exists("/etc/kalastack/vhosts/sites-available/" . $sitename)) {
    // Make sure a site is actually where it is supposed to be
    if (is_dir("/var/www/" . $sitename)) {
      drush_shell_exec("cp /etc/nginx/sites-available/drupaldefault /tmp/" . $sitename);
      $vhost_default_string = kala_get_file("/tmp/" . $sitename);

      //  Load up default drupal vhost and update with new site info
      $search = array("drupal.kala", "/var/www/drupal", "# THIS IS MEANT TO BE USED FOR BUILDING OTHER DRUPAL VHOSTS");
      $replace = array($sitename . ".kala", "/var/www/" . $sitename, "");
      $vhost_default_string = str_replace($search, $replace, $vhost_default_string);

      // Write the file and symlink it to sites-enabled
      kala_write_file("/tmp/" . $sitename, $vhost_default_string);
      drush_shell_exec ("cp /tmp/" . $sitename . " /etc/kalastack/vhosts/sites-available/" . $sitename);
      drush_shell_exec ("ln -s /etc/kalastack/vhosts/sites-available/" . $sitename . " /etc/kalastack/vhosts/sites-enabled/" . $sitename);

      // Restart nginx so the site actually is active
      drush_shell_exec ("service nginx restart");
      kala_update_etc_hosts($sitename . ".kala");
      drush_log('New vhost created!', 'success');
    }
  }
  else {
    // File already exists!
    drush_log('Seriously, the vhost file already exists!', 'warning');
  }
}

/** LOCAL ALIAS FILE STUFF **/
/**
 * Build a local alias
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 */
function terminatur_add_alias($site) {
  // Load up site but only if not passed in
  $args = drush_get_arguments();
  if (!is_array($site)) {
    $site = _kala_build_site($args[1]);
  }

  // Create an aliases file if there are no local sites
  if (!file_exists("/etc/kalastack/aliases/aliases.drushrc.php")) {
    file_put_contents("/etc/kalastack/aliases/aliases.drushrc.php", "");
    $aliases = array();
  }

  // Load the kala aliases file
  require '/etc/kalastack/aliases/aliases.drushrc.php';

  // Restore to first made first listed
  $aliases = array_reverse($aliases);
  if (!isset($aliases[$site['site'] . '.kala'])) {
    // Build the local alias
    $aliases[$site['site'] . '.kala'] = array(
      'root' => '/var/www/' . $site['site'],
      'uri' => $site['site'] . '.kala',
      'read-name' => $site['site-name'],
    );
    // If built from somewhere, indicate that here
    if ($site['built-from']) {
      $aliases[$site['site'] . '.kala']['built-from'] = $site['built-from'];
    }
  }

  // Sort so that last added is listed first
  $aliases = array_reverse($aliases);

  // Build output
  $output = "<?php \n\n";
  foreach ($aliases as $key => $value) {
    $output .= "\$aliases['" . $key . "'] = array(\n";
    $output .= "  'root' => '" . $value['root'] . "',\n";
    $output .= "  'uri' => '" . $value['uri'] . "',\n";
    $output .= "  'read-name' => '" . addslashes($value['read-name']) .  "',\n";
    // Only include built froms if they exist
    if ($value['built-from']) {
      $output .= "  'built-from' => '" . $value['built-from'] .  "',\n";
    }
    $output .= ");\n";
  }

  // Write file
  file_put_contents("/etc/kalastack/aliases/aliases.drushrc.php", $output);
}

/**
 * Remove a local alias
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 */
function terminatur_remove_alias($site) {
  // Load up site but only if not passed in
  $args = drush_get_arguments();
  if (!is_array($site)) {
    $site = _kala_build_site($args[1]);
  }

  // Load alias file if it exists
  if (file_exists("/etc/kalastack/aliases/aliases.drushrc.php")) {
    require '/etc/kalastack/aliases/aliases.drushrc.php';
  }
  else {
    return;
  }

  // Load the kala aliases file
  if (isset($aliases[$site['site'] . '.kala'])) {
    unset($aliases[$site['site'] . '.kala']);
  }

  // Build output
  $output = "<?php \n\n";
  foreach ($aliases as $key => $value) {
    $output .= "\$aliases['" . $key . "'] = array(\n";
    $output .= "  'root' => '" . $value['root'] . "',\n";
    $output .= "  'uri' => '" . $value['uri'] . "',\n";
    $output .= "  'read-name' => '" . addslashes($value['read-name']) .  "',\n";
    $output .= "  'built-from' => '" . $value['built-from'] .  "',\n";
    $output .= ");\n";
  }

  // Write file
  file_put_contents("/etc/kalastack/aliases/aliases.drushrc.php", $output);
}


