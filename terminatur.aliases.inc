<?php

 /**
 * @file
 *  aliases.inc: Helper functions to manipulate local alias settings
 */

// Constants
define('TERMINATUR_LOCAL_ALIASES_FILE', '/etc/drush/local.aliases.drushrc.php'); // @todo: might want to get this location in an OS agnostic way?

/**
 * Build a local alias
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 */
function _terminatur_aliases_add_alias($site, $destination) {
  // Create an aliases file if there are no local sites
  if (!file_exists(TERMINATUR_LOCAL_ALIASES_FILE)) {
    file_put_contents(TERMINATUR_LOCAL_ALIASES_FILE, "");
    $aliases = array();
  }
  // Define suffix depending on environment
  $suffix = KALABOX ? '.kala' : '.local';

  // Load the local aliases file
  require TERMINATUR_LOCAL_ALIASES_FILE;

  // Restore to first made first listed
  $aliases = array_reverse($aliases);
  if (!isset($aliases[$site['machine-name'] . $suffix])) {
    // Build the local alias
    $aliases[$site['machine-name'] . $suffix] = array(
      'root' => $destination . $site['machine-name'],
      'uri' => $site['machine-name'] . $suffix,
      'read-name' => $site['machine-name'],
    );
    // If built from somewhere, indicate that here
    if ($site['built-from']) {
      $aliases[$site['machine-name'] . $suffix]['built-from'] = $site['built-from'];
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
  file_put_contents(TERMINATUR_LOCAL_ALIASES_FILE, $output);
}

/**
 * Remove a local alias
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 */
function _terminatur_aliases_remove_alias($site, $destination) {
  // Load alias file if it exists
  if (file_exists(TERMINATUR_LOCAL_ALIASES_FILE)) {
    $aliases = array();
    require TERMINATUR_LOCAL_ALIASES_FILE;
  }
  else {
    return;
  }

  // Define suffix depending on environment
  $suffix = KALABOX ? '.kala' : '.local';

  // Load the kala aliases file
  if (isset($aliases[$site['machine-name'] . $suffix])) {
    unset($aliases[$site['machine-name'] . $suffix]);
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
  file_put_contents(TERMINATUR_LOCAL_ALIASES_FILE, $output);
}

/**
 * Return an array with local site data
 */
function _terminatur_aliases_get_local_site($machine_name) {
  // Load alias file if it exists
  if (file_exists(TERMINATUR_LOCAL_ALIASES_FILE)) {
    $aliases = array();
    require TERMINATUR_LOCAL_ALIASES_FILE;
  }
  else {
    return FALSE;
  }
  // Define suffix depending on environment
  $suffix = KALABOX ? '.kala' : '.local';
  return $aliases[$machine_name . $suffix];
}
