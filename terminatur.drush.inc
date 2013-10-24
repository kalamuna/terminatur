<?php
 /**
 * @file
 *  Terminatur: Drush commands to build pantheon sites locally
 */

// Constants
define('TERMINATUR_TMP_DIR', sys_get_temp_dir() . "/");
define('TERMINATUR_ENV', _terminatur_get_environment());

// Load up environment dependent stuff. Right now this is basiaclly just for
// kalastack/kalabox
// @todo: Eventually we will want to provide some basic handling for
// MAMP/Meglodon/etc
include_once('environment/terminatur.' . TERMINATUR_ENV . '.inc');

// Now that we have an environment load up other core stuff
include_once('terminatur.bootstrap.inc');
include_once('terminatur.code.inc');
include_once('terminatur.database.inc');
include_once('terminatur.files.inc');
include_once('terminatur.settings.inc');
include_once('terminatur.utils.inc');

/**
 * Implementation of hook_drush_command().
 *
 * @See drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function terminatur_drush_command() {
  $items = array();

  // Some generic options needed for DB based commands
  $db_options = array(
    'user' => array(
      'description' => 'A MySQL user with creds to create DBz.',
      'example-value' => 'root',
    ),
    'password' => array(
      'description' => 'The password for the MySQL user.',
      'example-value' => 'password',
    ),
    'host' => array(
      'description' => 'The host of the MySQL server.',
      'example-value' => 'localhost',
    ),
    'port' => array(
      'description' => 'The port of the MySQL server.',
      'example-value' => '3306',
    ),
  );

  $items['terminatur-code'] = array(
    'description' => dt('Pulls down the code for a Pantheon site.'),
    'aliases' => array('pullcode', 'pc'),
    'examples' => array(
      'drush pullcode mysite.dev' => 'Pulls down the code for the site at @pantheon.mysite.dev.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  $items['terminatur-data'] = array(
    'description' => dt('Pulls down the database for a Pantheon site.'),
    'aliases' => array('pulldata', 'pd'),
    'examples' => array(
      'drush pulldata sitename.dev' => 'Pulls down the database for a site at @pantheon.mysite.dev.',
    ),
    'options' => array(
      'pipe' => array(
        'description' => 'Use a pipe instead of a backup. This is slower but it will grab the most up to date data.',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['terminatur-data']['options'] += $db_options;

  $items['terminatur-files'] = array(
    'description' => dt('Pulls down the files for a Pantheon site.'),
    'aliases' => array('pullfiles', 'pf'),
    'examples' => array(
      'drush pullfiles sitename.dev' => 'Pulls down the files for a site at @pantheon.mysite.dev.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  $items['terminatur-pullsite'] = array(
    'description' => dt('Pulls down the code, data and files for a Pantheon site. If on Kalabox creates hosts entry, vhost, alias and edits settings.php.'),
    'aliases' => array('pullsite', 'ps'),
    'examples' => array(
      'drush pullsite sitename.dev' => 'Pulls down the code and data for a site at @pantheon.mysite.dev. Sets up server side conf on Kalabox.',
      'drush pullsite sitename.dev --files' => 'Pulls down the code, data and files for a site at @pantheon.mysite.dev. Sets up server side conf on Kalabox.',
    ),
    'options' => array(
      'pipe' => array(
        'description' => 'Use a pipe instead of a backup. This is slower but it will grab the most up to date data.',
      ),
      'files' => array(
        'description' => 'Use this flag to also pull down your files.',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['terminatur-pullsite']['options'] += $db_options;

  $items['terminatur-crushsite'] = array(
    'description' => dt('Removes a site completely.'),
    'aliases' => array('crushsite', 'crush', 'wmd', 'cs'),
    'examples' => array(
      'drush crushsite sitename.dev' => 'Removes a site at @pantheon.mysite.dev or @kalabox.mysite.dev completely.',
     ),
    'options' => array(),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['terminatur-crushsite']['options'] += $db_options;

  $items['terminatur-newsite'] = array(
    'description' => dt('Builds a new site locally.'),
    'aliases' => array('newsite', 'ns'),
    'examples' => array(
      'drush newsite sitename' => 'Builds a new Panopoly site called sitename.',
      'drush newsite sitename --profile=drupal7' => 'Builds a new vanilly Drupal 7 site called sitename.',
      'drush newsite sitename --profile=openatrium --site-name="My Awesome Site"' => 'Builds a new Open Atrium site called sitename with human readable name "My Awesome Site".',
    ),
    'options' => array(
      'profile' => array(
        'description' => 'Which distribution to install (use drupal7 for regular Drupal).',
        'example-value' => 'panopoly,openatrium,drupal7',
      ),
      'site-name' => array(
        'description' => 'Human readable meta data',
        'example-value' => '"My Awesome Site",BeastMode',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['terminatur-newsite']['options'] += $db_options;

  // Include standard arguments for above commands
  $common_args = array(
    'sitename' => 'The sitename.dev part of @pantheon.sitename.dev.',
  );
  $common_options = array(
    'destination' => array(
      'description' => 'The destination of your webroot.',
      'example-value' => '/var/www/',
    ),
  );
  foreach ($items as $key => $array) {
    if (isset($array['arguments'])) {
      $items[$key]['arguments'] = array_merge($items[$key]['arguments'], $common_args);
    }
    else {
      $items[$key]['arguments'] = $common_args;
    }
    if (isset($array['options'])) {
      $items[$key]['options'] = array_merge($items[$key]['options'], $common_options);
    }
    else {
      $items[$key]['options'] = $common_options;
    }
  }

  $items['terminatur-aliases'] = array(
    'description' => dt('Grabs your Pantheon aliases and parses them for use with terminatur.'),
    'aliases' => array('talias', 'ta'),
    'examples' => array(
      'drush talias arnold@terminatur.com ' => 'Grabs your Pantheon aliases and Terminates them.',
     ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  $items['terminatur-set-git'] = array(
    'description' => dt('Sets your gitconfig with your Pantheon profile info'),
    'aliases' => array('tset'),
    'examples' => array(
      'drush tset' => 'Grabs your Pantheon aliases and Terminates them.',
     ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  return $items;
}

/**
 * Gets the codebase for the Pantheon site
 *
 * @todo: Right now this assumes you have git, maybe STFP or WGET in the future?
 *
 * @param  boolean $sitename This is the "mysite.dev" part of @pantheon.mysite.dev
 */
function drush_terminatur_code($sitename = FALSE) {
  // Grabs info about the site
  if (!$sitename) {
    $sitename = drush_get_option('sitename');
  }
  if (!$site = _terminatur_get_site($sitename)) {
    return;
  }

  // Gets the webroot
  $destination = drush_get_option('destination');
  if (!$destination) {
    $destination = TERMINATUR_DEFAULT_WEBROOT;
  }

  drush_log(dt("Downloading code..."), 'warning');

  // Build code callback function
  $get_code_func = '_terminatur_code_' . $site['terminatur']['code']['method'];
  if (!$get_code_func($site, $destination)) {
    return;
  }

  drush_log(dt("Code downloaded."), 'success');
}

/**
 * Gets the database for the Pantheon site
 *
 * @param  boolean $sitename This is the "mysite.dev" part of @pantheon.mysite.dev
 */
function drush_terminatur_data($sitename = FALSE) {
  // Grabs info about the site
  if (!$sitename) {
    $sitename = drush_get_option('sitename');
  }
  if (!$site = _terminatur_get_site($sitename)) {
    return;
  }

  // Gets some options or sets defaults
  $destination = drush_get_option('destination');
  if (!$destination) {
    $destination = TERMINATUR_DEFAULT_WEBROOT;
  }
  $db_user = drush_get_option('user');
  if (!$db_user) {
    $db_user = TERMINATUR_DEFAULT_DB_USER;
  }
  $db_pass = drush_get_option('password');
  if (!$db_pass) {
    $db_pass = TERMINATUR_DEFAULT_DB_PASS;
  }
  $db_host = drush_get_option('host');
  if (!$db_host) {
    $db_host = TERMINATUR_DEFAULT_DB_HOST;
  }
  $db_port = drush_get_option('port');
  if (!$db_port ) {
    $db_port  = TERMINATUR_DEFAULT_DB_PORT;
  }
  $pipe = drush_get_option('pipe');

  drush_log(dt("Downloading data..."), 'warning');

  // Build code callback function
  $get_data_func = '_terminatur_data_' . $site['terminatur']['database']['method'];
  if (!$get_data_func($site, $destination)) {
    return;
  }

  drush_log(dt("Data downloaded."), 'success');
}

/**
 * Downloads or refreshs pantheon files
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 * @return if fails
 */
function drush_terminatur_files($sitename = FALSE) {
  // Grabs info about the site
  if (!$sitename) {
    $sitename = drush_get_option('sitename');
  }
  if (!$site = _terminatur_get_site($sitename)) {
    return;
  }

  // Gets some options or sets defaults
  $destination = drush_get_option('destination');
  if (!$destination) {
    $destination = TERMINATUR_DEFAULT_WEBROOT;
  }

  drush_log(dt("Downloading files..."), 'warning');

  // Build files callback function
  $get_files_func = '_terminatur_files_' . $site['terminatur']['files']['method'];
  if (!$get_files_func($site, $destination)) {
    return;
  }

  drush_log(dt("Files downloaded."), 'success');
}

/**
 * Create a new and fully instantiated site or build one that already exist on pantheon
 * @return when complete
 */
function drush_terminatur_pullsite($sitename = FALSE) {
  // Grabs info about the site
  if (!$sitename) {
    $sitename = drush_get_option('sitename');
  }
  if (!$site = _terminatur_get_site($sitename)) {
    return;
  }

  // Need MySQL
  if ($site['terminatur']['database']['method'] != 'mysql') {
    drush_log(dt("You need at least MySQL for this to work."), 'error');
    return;
  }

  // Gets some options or sets defaults
  $destination = drush_get_option('destination');
  if (!$destination) {
    $destination = TERMINATUR_DEFAULT_WEBROOT;
  }
  $db_user = drush_get_option('user');
  if (!$db_user) {
    $db_user = TERMINATUR_DEFAULT_DB_USER;
  }
  $db_pass = drush_get_option('password');
  if (!$db_pass) {
    $db_pass = TERMINATUR_DEFAULT_DB_PASS;
  }
  $db_host = drush_get_option('host');
  if (!$db_host) {
    $db_host = TERMINATUR_DEFAULT_DB_HOST;
  }
  $db_port = drush_get_option('port');
  if (!$db_port ) {
    $db_port  = TERMINATUR_DEFAULT_DB_PORT;
  }
  $pipe = drush_get_option('pipe');
  $files = drush_get_option('files');

// Build callback functions
  $get_data_func = '_terminatur_data_' . $site['terminatur']['database']['method'];
  $get_code_func = '_terminatur_code_' . $site['terminatur']['code']['method'];

  // Code & Data
  if (!$get_code_func($site, $destination)) {
    return;
  }
  if (!$get_data_func($site, $destination, $db_user, $db_pass, $db_host, $db_port, $pipe)) {
    return;
  }

  // If applicable, files
  if ($files) {
    $get_files_func = '_terminatur_files_' . $site['terminatur']['files']['method'];
    if (!$get_files_func($site, $destination)) {
      return;
    }
  }
  else {
    // We need these directories set up for cache clear!
    _terminatur_files_set_up_dirs($site, $destination);
  }

  // Build vhost add callback function
  $get_vhost_add_func = '_terminatur_vhost_add_' . TERMINATUR_ENV;
  $get_vhost_add_func($site['machine-name'], $destination);

  // Build host add callback function
  $get_host_add_func = '_terminatur_host_add_' . TERMINATUR_ENV;
  $get_host_add_func($site['machine-name'] . TERMINATUR_LOCAL_SUFFIX);

  // Clear cache
  drush_invoke_process('@' . TERMINATUR_LOCAL_PREFIX . '.' . $site['machine-name'] . TERMINATUR_LOCAL_SUFFIX, 'cache-clear', array('all'), array('y'));

  // Run additional environnment specific pullsite tasks
  if (function_exists($get_pull_more_func = '_terminatur_pullsite_' . TERMINATUR_ENV)) {
    $get_pull_more_func($site);
  }

  // Finish
  drush_log('Site built!', 'success');
}

/**
 * Obliterate a local site
 */
function drush_terminatur_crushsite($sitename = FALSE) {
  // Grabs info about the site
  if (!$sitename) {
    $sitename = drush_get_option('sitename');
  }
  if (!$site = _terminatur_get_site($sitename, TRUE)) {
    return false;
  }

  // Do a different check here because a local site does not have a DB method set
  if (!drush_shell_exec('mysql --version')) {
    drush_log(dt("You need at least MySQL for this to work."), 'error');
    return;
  }

  // Gets some options or sets defaults
  $destination = drush_get_option('destination');
  if (!$destination) {
    $destination = TERMINATUR_DEFAULT_WEBROOT;
  }
  $db_user = drush_get_option('user');
  if (!$db_user) {
    $db_user = TERMINATUR_DEFAULT_DB_USER;
  }
  $db_pass = drush_get_option('password');
  if (!$db_pass) {
    $db_pass = TERMINATUR_DEFAULT_DB_PASS;
  }
  $db_host = drush_get_option('host');
  if (!$db_host) {
    $db_host = TERMINATUR_DEFAULT_DB_HOST;
  }
  $db_port = drush_get_option('port');
  if (!$db_port ) {
    $db_port  = TERMINATUR_DEFAULT_DB_PORT;
  }

  drush_log(dt("Removing site..."), 'warning');

  // @todo: to be consistence maybe we want these to be abstracted out a bit?
  _terminatur_code_remove($site, $destination);
  _terminatur_data_remove($site, $destination, $db_user, $db_pass, $db_host, $db_port);
  _terminatur_files_remove($site, $destination);

  // Build vhost remove callback function
  $get_vhost_remove_func = '_terminatur_vhost_remove_' . TERMINATUR_ENV;
  $get_vhost_remove_func($site['machine-name']);

  // Build host remove callback function
  $get_host_remove_func = '_terminatur_host_remove_' . TERMINATUR_ENV;
  // If we don't check for this and pass in just the suffix we can wipe
  // needed entries
  if (isset($site['machine-name'])) {
    $get_host_remove_func($site['machine-name'] . TERMINATUR_LOCAL_SUFFIX);
  }

  // Remove local alias
  // Build alias callback function
  $get_alias_func = '_terminatur_alias_remove_' . TERMINATUR_ENV;
  $get_alias_func($site['machine-name']);

  // Done
  drush_log('Site removed.', 'success');
}

/**
 * Obliterate a local site
 */
function drush_terminatur_newsite($sitename = FALSE) {
  // Grabs info about the site
  if (!$sitename) {
    $sitename = drush_get_option('sitename');
  }
  if (preg_match('/[^a-z0-9-]/', $sitename)) {
    drush_log(dt("New sitename can only be lowercase letters, numbers and hyphens."), 'error');
    return;
  }

  // Do a different check here because a local site does not have a DB method set
  if (!drush_shell_exec('mysql --version')) {
    drush_log(dt("You need at least MySQL for this to work."), 'error');
    return;
  }

  // Gets some options or sets defaults
  $destination = drush_get_option('destination');
  if (!$destination) {
    $destination = TERMINATUR_DEFAULT_WEBROOT;
  }
  $profile = drush_get_option('profile');
  if (!$profile) {
    $profile = TERMINATUR_DEFAULT_PROFILE;
  }
  $site_name = drush_get_option('site-name');
  if (!$site_name) {
    $site_name = TERMINATUR_DEFAULT_SITENAME;
  }
  $db_user = drush_get_option('user');
  if (!$db_user) {
    $db_user = TERMINATUR_DEFAULT_DB_USER;
  }
  $db_pass = drush_get_option('password');
  if (!$db_pass) {
    $db_pass = TERMINATUR_DEFAULT_DB_PASS;
  }
  $db_host = drush_get_option('host');
  if (!$db_host) {
    $db_host = TERMINATUR_DEFAULT_DB_HOST;
  }
  $db_port = drush_get_option('port');
  if (!$db_port ) {
    $db_port  = TERMINATUR_DEFAULT_DB_PORT;
  }
  // Set options for Vanilla D7
  $download = $profile;
  if ($profile == 'drupal7') {
    $download = 'drupal';
  }
  $install = $profile;
  if ($profile == 'drupal7') {
    $install = 'standard';
  }
  // This check has to happen here because we need $destination
  if (is_dir($destination . $sitename)) {
    drush_log(dt("Already a site there, try another name."), 'error');
    return;
  }

  drush_log(dt("Building new site..."), 'warning');

  // @todo: depending on what we do in the future, maybe NEWSITE creation should be abstracted out as well?
  // This should verify a DB connection and create a DB
  // We need to run this first before there is a settings.php
  // We will run it again later to validate a new settings file
  // We also need to build a faux site array to get it to do what we want.
  $site = array();
  $site['machine-name'] = $sitename;
  $databases = _terminatur_data_get_db_creds($site, $destination, $db_user, $db_pass, $db_host, $db_port);
  // Download the profile
  // Set download command arguments
  $dl_args = array($download);
  // DL options
  $dl_options = array(
    'destination' => $destination,
    'drupal-project-rename' => $sitename,
    'y' => TRUE,
  );
  drush_invoke_process('', 'pm-download', $dl_args, $dl_options);

  // Install options
  $db_url = 'mysql://' . $databases['default']['default']['username'] . ':' . $databases['default']['default']['password'] . '@' . $databases['default']['default']['host'] . ':' . $databases['default']['default']['port'] . '/' . $databases['default']['default']['database'];
  $account_pass = TERMINATUR_DEFAULT_USERPASS;

  // Sadly have to do it this way because site-install doesnt let us specify a dir
  // @todo: add in more options for admin name/email, etc
  drush_shell_exec("cd " . $destination . $sitename . " && drush site-install " . $install . " -y --db-url=" . $db_url . " --account-pass=" . $account_pass . " --site-name='" . $site_name . "'");

  // Update the new settings file
  $databases = _terminatur_data_get_db_creds($site, $destination, $db_user, $db_pass, $db_host, $db_port);

  // Build vhost add callback function
  $get_vhost_add_func = '_terminatur_vhost_add_' . TERMINATUR_ENV;
  $get_vhost_add_func($sitename, $destination);

  // Build host add callback function
  $get_host_add_func = '_terminatur_host_add_' . TERMINATUR_ENV;
  $get_host_add_func($sitename . TERMINATUR_LOCAL_SUFFIX);

  // Clear cache
  drush_invoke_process('@' . TERMINATUR_LOCAL_PREFIX . '.' . $sitename . TERMINATUR_LOCAL_SUFFIX, 'cache-clear', array('all'), array('y'));

  // Finish
  drush_log('New site built!', 'success');
}

/**
 * Refresh and reparse Pantheon alias file.
 */
function drush_terminatur_aliases() {
  return terminatur_bootstrap(TRUE);
}

/**
 * Set's your gitconfig with Pantheon profile info
 */
function drush_terminatur_set_git() {
  // Do a different check here because a local site does not have a DB method set
  if (!drush_shell_exec('git --version')) {
    drush_log(dt("You need git for this shit."), 'error');
    return;
  }
  $user = array();
  $user = _terminatur_get_user();

  // Set git config
  drush_shell_exec("git config --global user.name \"" . $user->profile->firstname . " " . $user->profile->lastname . "\"");
  drush_shell_exec("git config --global user.email " . $user->email);

  // Yay!
  drush_log('Git username and email have been set!', 'success');
}

/**
 * Determine what environmental file to load
 */
function _terminatur_get_environment() {
  // Check for Kalastack
  if (isset($_SERVER['KALABOX']) && $_SERVER['KALABOX'] === 'on') {
    define('KALABOX', TRUE);
    return 'kalastack';
  }

  // elseif @todo some check for mamp
  // elseif @todo some check for meglodon
  // elseif @todo some check for devstack
  // elseif @todo some check for drupalpro
  else {
    return 'default';
  }
}
