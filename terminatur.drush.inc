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
include_once('terminatur.aliases.inc');

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

  $items['terminatur-code'] = array(
    'description' => dt('Pulls down the code for a Pantheon site.'),
    'aliases' => array('pullcode', 'pc'),
    'examples' => array(
      'drush pullcode mysite.dev' => 'Pulls down the code for the site at @pantheon.mysite.dev.',
    ),
    'options' => array(
      'destination' => array(
        'description' => 'The destination of your webroot.',
        'example-value' => '/var/www/',
      ),
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
      'destination' => array(
        'description' => 'The destination of your webroot.',
        'example-value' => '/var/www/',
      ),
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
      'pipe' => array(
        'description' => 'Use a pipe instead of a backup. This is slower but it will grab the most up to date data.',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  $items['terminatur-files'] = array(
    'description' => dt('Pulls down the files for a Pantheon site.'),
    'aliases' => array('pullfiles', 'pf'),
    'options' => array(
      'destination' => array(
        'description' => 'The destination of your webroot.',
        'example-value' => '/var/www/',
      ),
    ),
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
      'files' => array(
        'description' => 'Use this flag to also pull down your files.',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  $items['terminatur-refreshsite'] = array(
    'callback' => 'terminatur_pullsite',
    'description' => dt('Refreshes the code, data and files for a Pantheon site.'),
    'aliases' => array('refreshsite', 'rs'),
    'options' => array(
      'destination' => array(
        'description' => 'The destination of your webroot.',
        'example-value' => '/var/www/',
      ),
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
    ),
    'examples' => array(
      'drush refreshsite sitename.dev' => 'Refreshes the code, data and files for a site at @pantheon.mysite.dev.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  $items['terminatur-crushsite'] = array(
    'description' => dt('Removes a site completely.'),
    'aliases' => array('crushsite', 'crush', 'wmd', 'cs'),
    'examples' => array(
      'drush crushsite sitename.dev' => 'Removes a site at @pantheon.mysite.dev or @kalabox.mysite.dev completely.',
     ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  // Include standard arguments for above commands
  $common_args = array(
    'sitename' => 'The sitename.dev part of @pantheon.sitename.dev.',
  );
  foreach ($items as $key => $array) {
    if (isset($array['arguments'])) {
      $items[$key]['arguments'] = array_merge($items[$key]['arguments'], $common_args);
    }
    else {
      $items[$key]['arguments'] = $common_args;
    }
  }

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

  $items['terminatur-aliases'] = array(
    'description' => dt('Grabs your Pantheon aliases and parses them for use with terminatur.'),
    'aliases' => array('talias', 'ta'),
    'examples' => array(
      'drush talias arnold@terminatur.com --password=illbeback' => 'Deletes the Pantheon site. Must use a sitename alias (format "sitename.dev") derived from a valid Pantheon alias as first argument.',
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
  $sites = terminatur_bootstrap();
  $site = $sites[$sitename];
    if (!$site) {
    drush_log(dt("Not a valid Pantheon site."), 'error');
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
  $get_code_func($site, $destination);
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
  $sites = terminatur_bootstrap();
  $site = $sites[$sitename];
  if (!$site) {
    drush_log(dt("Not a valid Pantheon site."), 'error');
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

  // Build data callback function
  $get_data_func = '_terminatur_data_' . $site['terminatur']['database']['method'];
  $get_data_func($site, $destination, $db_user, $db_pass, $db_host, $db_port, $pipe);
}

/**
 * Downloads or refreshs pantheon files
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 * @return if fails
 */
function drush_terminatur_files($sitename = FALSE) {
  if (!$sitename) {
    $sitename = drush_get_option('sitename');
  }
  $sites = terminatur_bootstrap();
  $site = $sites[$sitename];

  // Only run if its a panthoen site
  if ($site['pantheon'] === TRUE) {
    // Checking ot see ig the files firectory exists already
    drush_print('We are now in the process of checking to see whether or not you have tried to sync these here filez before!');
    if (is_dir("/var/www/" . $site['site'] . "/sites/default/files")) {
      drush_log('you have!', 'success');
    }
    // Build the files directory and set its permissions if it doesnt exist yet
    else {
      mkdir("/var/www/" . $site['site'] . "/sites/default/files");
      mkdir("/var/www/" . $site['site'] . "/sites/default/private");
      drush_log('Building up some sweet sweet files directories for you', 'success');
      // Settings this to 777 for now because its the only way to get file uploads to work properly with nfs sharing
      drush_shell_exec("chmod 777 /var/www/" . $site['site'] . "/sites/default/files");
      drush_shell_exec("chmod 777 /var/www/" . $site['site'] . "/sites/default/files/private");
    }

    // RSYNCing files from pantheon
    drush_log('Doing that funky file sync tango. It feels good. Please wait...', 'success');
    drush_shell_exec("rsync -rlztv --ipv4 -e 'ssh " . $site['ssh'] . "' --exclude 'js' --exclude 'css' --exclude 'ctools' --exclude 'imagecache' --exclude 'xmlsitemap' --exclude 'backup_migrate' --exclude 'styles' --exclude 'less' " . $site['env'] . "." . $site['id'] . "@appserver." . $site['env'] . "." . $site['id'] . ".drush.in:files /var/www/" . $site['site'] . "/sites/default/");
    drush_log('Consider your files N*SYNC', 'success');
  }
  else {
    return;
  }
}

/**
 * Create a new and fully instantiated site or build one that already exist on pantheon
 * @return when complete
 */
function drush_terminatur_pullsite($sitename = FALSE) {
  if (!$sitename) {
    $sitename = drush_get_option('sitename');
  }
  $sites = terminatur_bootstrap();
  $site = $sites[$sitename];

  // Require git/mysql/rsync

  // Grab arguments and options
  $args = drush_get_arguments();
  $profile = drush_get_option('profile');
  $files = drush_get_option('files');
  $site_name = drush_get_option('site-name');

  // Set a default profile if not set
  if (!isset($profile)) {
    $profile = 'panopoly';
  }

  // Build an array of info for the site
  $site = _kala_build_site($args[1]);

  // START IT UP!
  drush_print('WE GONE BUILD THIS HERE SITE NOW! STANDBY LADDY/LASS');

  // Build a pantheon site
  if (isset($site['pantheon']) && $site['pantheon'] === TRUE) {
    // Get the code
    kala_code($site);
    drush_log('Codebase: Qapla\'!', 'success');

    // Add some settings for kalabox
    _kala_reset_settings($site);
    drush_log('Settings: Rebuilt!, Check git status', 'success');

    // Get the data
    kala_data($site);
    drush_log('Database: Qapla\'!', 'success');

    // Get the files but only run if option is set to do so
    if ($files) {
      kala_files($site);
      drush_log('Files: Qapla\'!', 'success');
    }
  }
  // Spin up a new site
  // Vhost and cleanupz
  drush_shell_exec("sudo drush vhost " . $args[1]);
  drush_shell_exec("cd /var/www/" . $site['site'] . " && drush cc all -y");
  drush_log('Drush: We clear yo cache', 'success');
  _kala_add_alias($site);
  drush_log('Success: ' . $site['site-name'] .   ' has been built!', 'success');

  return;
}

/**
 * Obliterate a local site
 */
function drush_terminatur_crushsite($sitename = FALSE) {
  if (!$sitename) {
    $sitename = drush_get_option('sitename');
  }
  $sites = terminatur_bootstrap();
  $site = $sites[$sitename];

  // If that site actually exists...
  if (isset($site['site'])) {
    drush_print('PREPARE TO RIDE ON EL CRUSH BUS');

    // Remove Code and Files
    $web_root = "/var/www/" . $site['site'];
    // Make write-protected directory and files writable so we can remove them.
    chmod($web_root . '/sites/default', 0755);
    chmod($web_root . '/sites/default/default.settings.php', 0644);
    chmod($web_root . '/sites/default/settings.php', 0644);
    // Remove the webroot and everything in it.
    if (drush_shell_exec("rm -rf " . $web_root)) {
      drush_log('CODES/FILES = FUCKING CRUSHED', 'success');
    }

    // Remove Data
    if (drush_shell_exec("mysql -uroot -ppassword -hlocalhost -e \"drop database " . $site['kaladb'] . "\"")) {
      drush_log('DB = FUCKING CRUSHED', 'success');
    }

    // Remove VHOST
    if (drush_shell_exec("sudo rm -rf /etc/kalastack/vhosts/sites-available/" . $site['site'])) {
      if (drush_shell_exec("sudo rm -rf /etc/kalastack/vhosts/sites-enabled/" . $site['site'])) {
        drush_log('VHOST = FUCKING CRUSHED', 'success');
      }
    }

    // Restart nginx
    drush_shell_exec("sudo service nginx restart");

    // Remove hosts entry
    kala_remove_etc_hosts($site['site'] . ".kala");
    _kala_remove_alias($site);
    drush_log('SITE = FUCKING CRUSHED', 'success');
  }
}

/**
 * Obliterate a local site
 */
function drush_terminatur_newsite() {
  if (!is_dir("/var/www/" . $site['site'])) {
    // Set sitename
    $site['site-name'] = $site_name ? $site_name : $site['site'];

    // Download distro in webroot
    $download = $profile;
    $download_message = 'We are now downloading the ' . $profile . ' distribution';
    if ($profile == 'drupal7') { // If vanilla Drupal 7.
      $download = 'drupal';
      $download_message = 'We are now downloading Drupal 7';
    }
    drush_log($download_message, 'success');
    drush_shell_exec("cd /var/www && drush dl " . $download . " --drupal-project-rename=" . $site['site'] . " -y");

    // Spin up empty DB
    if (drush_shell_exec("mysql -uroot -ppassword -hlocalhost -e \"create database " . $site['kaladb'] . "\"")) {
      drush_log('Dearest friend, a new database called ' . $site['kaladb'] . ' hath been created', 'success');
    }

    // Run the site install
    $install = $profile;
    if ($profile == 'drupal7') { // If vanilla Drupal 7.
      $install = 'standard';
    }
    drush_log('Building a new ' . $profile . 'site.', 'success');
    if (drush_shell_exec("cd /var/www/" . $site['site'] . " && drush site-install " . $install . " -y --db-url=mysql://root:password@localhost/" . $site['kaladb'] . " --account-pass=admin")) {
      drush_log('Build complete', 'success');
    }
  }
}

/**
 * Refresh and reparse Pantheon alias file.
 */
function drush_terminatur_aliases() {
  return terminatur_bootstrap(TRUE);
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


