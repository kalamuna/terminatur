<?php

 /**
 * @file
 *  aliases.inc: Helper functions to manipulate local alias settings
 */

/**
 * Build a local alias
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 */
function _terminatur_aliases_add_alias($site, $destination) {
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
  _terminatur_aliases_write_aliases($aliases);
}

/**
 * Remove a local alias
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 */
function _terminatur_aliases_remove_alias($machine_name) {
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
  _terminatur_aliases_write_aliases($aliases);
}

/**
 * Return an array with local site data
 */
function _terminatur_aliases_write_aliases($aliases) {
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
  return $aliases[$machine_name . TERMINATUR_LOCAL_SUFFIX];
}
