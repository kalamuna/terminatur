<?php
 /**
 * @file
 *  Terminatur: Drush commands to build pantheon sites locally
 */

// Constants
define('TERMINATUR_TMP_DIR', sys_get_temp_dir());
define('TERMINATUR_DEFAULT_WEBROOT', '/var/www/');
define('TERMINATUR_DEFAULT_DB_USER', 'root');
define('TERMINATUR_DEFAULT_DB_PASS', 'password');
define('TERMINATUR_DEFAULT_DB_HOST', 'localhost');

// Load up core stuff
# include_once('terminatur.bootstrap.inc');

// If using in conjunction with KALABOX load some other things
if (isset($_SERVER['KALABOX']) && $_SERVER['KALABOX'] === 'on') {
  include_once('terminatur.log.inc');
  include_once('terminatur.host.inc');
}

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
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

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
    'arguments' => array(
      'email' => 'Email address of dashboard account.',
    ),
    'options' => array(
      'password' => array(
        'description' => 'Optional: include password for script-friendly use.',
        'example-value' => 'mypassword',
      ),
    ),
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
function drush_terminatur_code($sitename = FALSE){
  // Grabs info about the site
  if (!$sitename) {
    $sitename = drush_get_option('sitename');
  }
  $sites = terminatur_bootstrap();
  $site = $sites[$sitename];

  // Gets the webroot
  $destination = drush_get_option('destination');
  if (!$destination) {
    $destination = TERMINATUR_DEFAULT_WEBROOT;
  }

  // @todo: some function to check for and return best download mode: GIT/SFTP/WGET?
  // Check if the codebase already exists
  if (is_dir($destination . $site['machine_name'])) {
    if (drush_shell_exec("cat " . $destination . $site['machine_name'] . "/.git/config | grep url")) {
      drush_shell_exec("cat " . $destination . $site['machine_name'] . "/.git/config | grep url > " . TERMINATUR_TMP_DIR . "gitconfig" . $site['machine_name']);
      $gitconfig_string = trim(file_get_contents(TERMINATUR_TMP_DIR . "gitconfig" . $site['machine_name']));
      drush_shell_exec("rm /tmp/gitconfig" . $site['machine_name']);

      // Codebase does exist, just refresh that code
      if ($gitconfig_string == "url = " . $site['git_connect']) {
        drush_shell_exec("git --git-dir=" . $destination . $site['machine_name'] . "/.git pull");
        drush_log(dt('Code refreshed!'), 'success');
        return;
      }
    }
  }
  // Git clone for the first time
  else {
    drush_shell_exec("git clone " . $site['git_connect']);
    drush_log(dt('Code cloned!'), 'success');
    return;
  }

  drush_log(dt('Looks like you already have stuff in the target directory. Remove it and try again!'), 'error');
}


/**
 * Gets the database for the Pantheon site
 *
 * @param  boolean $sitename This is the "mysite.dev" part of @pantheon.mysite.dev
 */
function drush_terminatur_data($sitename = FALSE){
  // Grabs info about the site
  if (!$sitename) {
    $sitename = drush_get_option('sitename');
  }
  $sites = terminatur_bootstrap();
  $site = $sites[$sitename];

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

  // Create a new database, silently fails if DB already exists
  if (!drush_shell_exec("mysql -u" . $db_user . " -p" . $db_pass . " -h" . $db_host . " -e \"create database " . $site['local_db'] . "\"")) {
    // @todo: log message
  }

  // Unreap my heart: make sure DB server is awake
  if (!drush_shell_exec("drush @pantheon." . $site['machine_name'] . "." . $site['env'] . " status")) {
    // @todo: log message
  }

  // Downloading and importing the data
  // Trying SQL-SYNC-PIPE first
  if (drush_shell_exec("cd " $destination . $site['machine_name'] . " && drush pipe @" . $site['machine_name'] . "." . $site['env'] . " -y --disable=pantheon_apachesolr --ssh-options='" . $site['ssh'] . "'")) {
  }
  // Using MYSQLDUMP Method if SSP fails
  else {
    if (!drush_shell_exec("mysqldump -u" . $site['db_user'] . " -h" . $site['db_host'] ." -P" . $site['db_port'] ." -p" . $site['db_pass'] . " -CceKqQ --add-drop-table --add-locks --create-options --no-autocommit --single-transaction " . $site['db_db'] ." > " . TERMINATUR_TMP_DIR . $site['uuid'] .".sql")) {
      // @todo: log message
    }
    if (!drush_shell_exec("mysql -u" . $db_user . " -p" . $db_pass . " -h" . $db_host . " " . $site['local_db'] . " < "TERMINATUR_TMP_DIR . $site['uuid'] .".sql")) {
      // @todo: log message
    }
  }
}

/**
 * Downloads or refreshs pantheon files
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 * @return if fails
 */
function drush_terminatur_files($sitename = FALSE){
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
  $aliases = terminatur_parse_aliases();
  //$aliases = terminus_bootstrap(TRUE);
}

/**
 * Parse Pantheon alias file for use with terminatur
 */
function terminatur_parse_aliases() {
  // Load in an unblemished Pantheon alias file
  $aliases = array();
  if (file_exists(TERMINATUR_TMP_DIR . "/pantheon.aliases.drushrc.php")) {
    require TERMINATUR_TMP_DIR . "/pantheon.aliases.drushrc.php";
  }
  else {
    return FALSE;
  }

  // Do bunch of fun stuff!
  foreach ($aliases as $key => $value) {
    // Remove all non-dev environments
    // @todo: eventually we want this to be all dev and multisite environments
    // if (strpos($key, '.test') !== FALSE || strpos($key, '.live') !== FALSE) {
    if (strpos($key, '.dev') === FALSE) {
      unset($aliases[$key]);
    }
    else {
      // Get sitename and environment
      $site_parts = explode('.', $key);
      $aliases[$key]['machine_name'] = $site_parts[0];
      $aliases[$key]['env'] = $site_parts[1];

      // Connect via terminus to get other fun stuff
      $aliases[$key]['uuid'] = terminus_get_site_uuid_by_name($aliases[$key]['sitename']);

      // Build other relevant things
      $aliases[$key]['local_db'] = str_replace("-", "_", $aliases[$key]['sitename'] . "_local");
      $aliases[$key]['ssh'] .= ' -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no';
      $aliases[$key]['built-from'] = $key;

      // Build DB stuff
      $regex = '/^mysql:\/\/([^:]+):([^@]+)@([^:]+):(\d+)\/(.+)$/';
      $matches = array();
      preg_match($regex, $aliases[$key]['db_url'], $matches);
      $aliases[$key]['db_user'] = $matches[1];
      $aliases[$key]['db_pass'] = $matches[2];
      $aliases[$key]['db_host'] = $matches[3];
      $aliases[$key]['db_port'] = $matches[4];
      $aliases[$key]['db_db'] = $matches[5];

      // Build Code optionz
      $aliases[$key]['git_connect'] = "ssh://codeserver.dev." . $aliases[$key]['uuid'] . "@codeserver.dev." . $aliases[$key]['uuid'] . ".drush.in:2222/~/repository.git";

    }
  }

  $test == $test;
}
