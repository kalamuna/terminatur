<?php
/**
 * @file
 * Initialization functionality and Pantheon API integration.
 */

define('TERMINATUR_SSH_KEY', 'terminatur.id_rsa');
define('ALIASES_FILE', '/etc/drush/pantheon.aliases.drushrc.php'); // @todo: might want to get this location in an OS agnostic way?

/**
 * Ensures user has set up SSH and has downloaded their Pantheon aliases file.
 *
 * If either isn't set up, it authenticates with Pantheon and carries out the
 * necessary setup steps.
 */
function terminatur_bootstrap($force_refresh = FALSE) {
  $pantheon_keys = drush_cache_get('pantheon-keys', 'terminatur');
  if (!$pantheon_keys) {
    // Check for a SSH key that works with Pantheon.
    $pantheon_keys = _terminatur_get_pantheon_keys();
    // If user doesn't have a SSH key set up with Pantheon, offer to make one for them.
    if (!$pantheon_keys) {
      $pantheon_keys = array(_terminatur_set_up_key());
    }
    drush_cache_set('pantheon-keys', $pantheon_keys, 'terminatur');
  }

  if (file_exists(getenv('HOME') . '/.ssh/' . TERMINATUR_SSH_KEY)) {
    drush_shell_exec('eval "$(ssh-agent)"');
    drush_shell_exec('ssh-add ' . getenv('HOME') . '/.ssh/' . TERMINATUR_SSH_KEY);
  }

  // Download and parase the aliases file if it's not there.
  // @todo: might want to do some sort of validation here first?
  $aliases = array();
  if (!file_exists(ALIASES_FILE) || $force_refresh === TRUE) {
    _terminatur_download_aliases();
    $aliases = _terminatur_parse_aliases();
  }
  else {
    require ALIASES_FILE;
  }

  // @todo Validate the pantheon aliases file.
  return $aliases;
}

/**
 * Returns data for the current Terminus session, authenticating if necessary.
 *
 * @return array
 *   Terminus session data.
 */
function _terminatur_get_session() {
  // Use current session if there is one.
  $session_data = terminus_bootstrap();
  if (isset($session_data['email']) && terminus_validate_session($session_data)) {
    return $session_data;
  }
  // Otherwise, authenticate with Terminus.
  // @todo: we need a way to do this that is not interactive for kalaboxage
  $email = drush_prompt(dt('Pantheon dashboard email'), NULL, TRUE);
  drush_terminus_pantheon_auth($email);
  // @todo Handle authentication failure.
  return terminus_bootstrap();
}

/**
 * Finds any keys on the host that are registered with Pantheon.
 *
 * @return array|bool
 *   An array of public keys registered with Pantheon, or false if there are none.
 */
function _terminatur_get_pantheon_keys() {
  $session_data = _terminatur_get_session();
  $user_id = $session_data['user_uuid'];
  // Get list of keys on Pantheon and keys on host.
  $pantheon_keys = terminus_api_user_sshkey_get($user_id);
  $pantheon_keys = array_values(drush_json_decode($pantheon_keys['json']));
  drush_shell_exec('ssh-add -L');
  $host_keys = drush_shell_exec_output();
  $pantheon_keys = array_map('_terminatur_extract_key', $pantheon_keys);
  $host_keys = array_map('_terminatur_extract_key', $host_keys);
  $intersection = array_intersect($host_keys, $pantheon_keys);
  if (!empty($intersection)) {
    return $intersection;
  }
  return FALSE;
}

/**
 * Extracts a public SSH key's body.
 *
 * @param string $key_data
 *   Data from the public key file.
 *
 * @return string|bool
 *   Key body or false if unable to extract it.
 */
function _terminatur_extract_key($key_data) {
  $matches = array();
  preg_match('/^ssh-(?:rsa|dsa)\s([^\s]+)\s.+$/', $key_data, $matches);
  if ($matches && isset($matches[1])){
    return $matches[1];
  }
  return FALSE;
}

/**
 * Generates a SSH key and adds it to the user's Pantheon account.
 *
 * @return string
 *   The generated public key.
 */
function _terminatur_set_up_key() {
  // @todo Ask permission via cli.
  $session_data = _terminatur_get_session();
  $user_id = $session_data['user_uuid'];
  // Generate a key on the host.
  // @todo Remove existing Terminatur ssh key if it exists.
  drush_shell_exec('hostname');
  $hostname = drush_shell_exec_output();
  $key_file = getenv('HOME') . '/.ssh/' . TERMINATUR_SSH_KEY;
  drush_shell_exec('ssh-keygen -C "terminatur@' . $hostname[0]  . '" -f "' . $key_file . '" -P ""');
  // Read generated key in and submit it to Terminus.
  $key = file_get_contents($key_file . '.pub');
  terminus_api_sshkey_validate($user_id, $key);
  // @todo Check key validation result.
  return $key;
}

/**
 * Calls Terminus to download the user's aliases file to /tmp.
 */
function _terminatur_download_aliases() {
  $session_data = _terminatur_get_session();
  drush_invoke_process('@self', 'pantheon-aliases', array(), array('destination' => TERMINATUR_TMP_DIR . 'terminatur.aliases.tmp'));
}

/**
 * Parse Pantheon alias file for use with terminatur
 */
function _terminatur_parse_aliases() {
  // Load in an unblemished Pantheon alias file
  $aliases = array();
  if (file_exists(TERMINATUR_TMP_DIR . "/terminatur.aliases.tmp")) {
    require TERMINATUR_TMP_DIR . "/terminatur.aliases.tmp";
  }
  else {
    return FALSE;
  }

  // Remove all non-dev environments
  // @todo: eventually we want this to be all dev and multisite environments
  // if (strpos($key, '.test') !== FALSE || strpos($key, '.live') !== FALSE) {
  foreach ($aliases as $key => $value) {
    if (strpos($key, '.dev') === FALSE) {
      unset($aliases[$key]);
    }
  }

  // Parse remaining aliases so they can be Terminatud
  foreach ($aliases as $key => $value) {
    // Get sitename and environment
    $site_parts = explode('.', $key);
    $aliases[$key]['machine-name'] = $site_parts[0];
    $aliases[$key]['env'] = $site_parts[1];
    // @todo: figure out how to handle multiple levels
    unset($aliases[$key]['path-aliases']);

    // Connect via terminus to get other fun stuff
    $aliases[$key]['uuid'] = terminus_get_site_uuid_by_name($aliases[$key]['machine-name']);

    // Build other relevant things
    $aliases[$key]['local-db'] = str_replace("-", "_", $aliases[$key]['machine-name'] . "_local");
    $aliases[$key]['ssh-options'] .= ' -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no';
    $aliases[$key]['built-from'] = $key;

    // Build DB stuff
    $regex = '/^mysql:\/\/([^:]+):([^@]+)@([^:]+):(\d+)\/(.+)$/';
    $matches = array();
    preg_match($regex, $aliases[$key]['db-url'], $matches);
    $aliases[$key]['db-user'] = $matches[1];
    $aliases[$key]['db-pass'] = $matches[2];
    $aliases[$key]['db-host'] = $matches[3];
    $aliases[$key]['db-port'] = $matches[4];
    $aliases[$key]['db-db'] = $matches[5];

    // Build Code optionz
    $aliases[$key]['git-connect'] = "ssh://codeserver.dev." . $aliases[$key]['uuid'] . "@codeserver.dev." . $aliases[$key]['uuid'] . ".drush.in:2222/~/repository.git " . $aliases[$key]['machine-name'];
  }

  // Create output string
  $output = "<?php \n";
  $output .= "// TERMINATUR PARSED PANTHEON ALIASES \n\n";
  foreach ($aliases as $site) {
    $test == $test;
    $output .= "\$aliases['" . $site['machine-name'] . "." . $site['env'] . "'] = array(\n";
    foreach ($site as $key => $value) {
      $output .= "  '" . $key . "' => '" . $value . "',\n";
    }
    $output .= ");\n";
  }

  // Write file
  file_put_contents(ALIASES_FILE, $output);

  // Return
  return $aliases;
}
