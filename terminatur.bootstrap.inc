<?php
/**
 * @file
 * Initialization functionality and Pantheon API integration.
 */

define('TERMINATUR_SSH_KEY', 'terminatur.id_rsa');
define('ALIASES_FILE', '/etc/drush/pantheon.aliases.drushrc.php');

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
  // Download the aliases file if it's not there.
  if (!file_exists(ALIASES_FILE)) {
    _terminatur_download_aliases();
  }
  // @todo Validate the aliases file.
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
  drush_invoke_process('@self', 'pantheon-aliases', array(), array('destination' => '/tmp/terminatur.aliases.tmp'));
}
