<?php
 /**
 * @file
 *  Terminatur: Drush commands to build pantheon sites locally
 */

/**
 * If using in conjunction with KALABOX load some other things
 */
if (isset($_SERVER['KALABOX']) && $_SERVER['KALABOX'] === 'on') {
  include_once('terminus.log.inc');
  include_once('terminus.host.inc');
}

/**
 * Implementation of hook_drush_command().
 *
 * @See drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function kala_drush_command() {
  $items = array();

  $items['kalacode'] = array(
    'callback' => 'kala_code',
    'description' => dt('Git clones your Pantheon site codebase'),
    'aliases' => array('code', 'kc'),
    'examples' => array(
      'drush code sitename.dev' => 'Git clone of your pantheon site. Must use a sitename alias (format "sitename.dev") derived from a valid Pantheon alias as first argument.',
    ),
    'arguments' => array(
      'site' => 'A sitename.dev, derived from a valid Pantheon site alias. Do not use the @pantheon convention for this alias as we are not using it as our site.',
    ),

    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['kaladata'] = array(
    'callback' => 'kala_data',
    'description' => dt('Imports your Pantheon site database'),
    'aliases' => array('data', 'kd'),
    'examples' => array(
      'drush data sitename.dev' => 'Import the DB of your pantheon site. Must use a sitename alias (format "sitename.dev") derived from a valid Pantheon alias as first argument.',
    ),
    'arguments' => array(
      'site' => 'A sitename.dev, derived from a valid Pantheon site alias. Do not use the @pantheon convention for this alias as we are not using it as our site.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['kalafiles'] = array(
    'callback' => 'kala_files',
    'description' => dt('Imports your Pantheon site files'),
    'aliases' => array('files', 'kf'),
    'examples' => array(
      'drush files sitename.dev' => 'Imports the files of your pantheon site. Must use a sitename alias (format "sitename.dev") derived from a valid Pantheon alias as first argument.',
    ),
    'arguments' => array(
      'site' => 'A sitename.dev, derived from a valid Pantheon site alias. Do not use the @pantheon convention for this alias as we are not using it as our site.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['kalabuild'] = array(
    'callback' => 'kala_build',
    'description' => dt('Imports your Pantheon site'),
    'aliases' => array('build', 'kb'),
    'examples' => array(
      'drush build sitename.dev' => 'Imports the code, data and files of your pantheon site. Must use a sitename alias (format "sitename.dev") derived from a valid Pantheon alias as first argument.',
    ),
    'arguments' => array(
      'site' => 'A sitename.dev, derived from a valid Pantheon site alias. Do not use the @pantheon convention for this alias as we are not using it as our site.',
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
      'files' => array(
        'description' => 'Use this flag to also download your files',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  $items['kalarefresh'] = array(
    'callback' => 'kala_build',
    'description' => dt('Refreses your Pantheon site'),
    'aliases' => array('krefresh', 'kr'),
    'examples' => array(
      'drush kr sitename.dev' => 'Refreshes the code, data and files of your pantheon site. Must use a sitename alias (format "sitename.dev") derived from a valid Pantheon alias as first argument.',
    ),
    'arguments' => array(
      'site' => 'A sitename.dev, derived from a valid Pantheon site alias. Do not use the @pantheon convention for this alias as we are not using it as our site.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['kaladestroy'] = array(
    'callback' => 'kala_destroy',
    'description' => dt('Removes a pantheon site'),
    'aliases' => array('destroy', 'crush', 'wmb', 'kd'),
    'examples' => array(
      'drush crush sitename.dev' => 'Deletes the Pantheon site. Must use a sitename alias (format "sitename.dev") derived from a valid Pantheon alias as first argument.',
     ),
    'arguments' => array(
      'site' => 'A sitename.dev, derived from a valid Pantheon site alias. Do not use the @pantheon convention for this alias as we are not using it as our site.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  return $items;
}


/**
 * Implementation of hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function kala_drush_help($section) {
  switch ($section) {
    case 'drush:kala':
      return dt("Shows useful info.");
    case 'drush:kalacode':
      return dt("Gets Pantheon codebase.");
    case 'drush:kaladata':
      return dt("Gets Pantheon database.");
    case 'drush:kalafiles':
      return dt("Gets Pantheon files.");
    case 'drush:kalabuild':
      return dt("Builds your Pantheon site.");
    case 'drush:kalabuild':
      return dt("Refreshes your Pantheon site.");
  }
}




/**
 * [kala_code description]
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 * @return only on error
 */
function kala_code($site = NULL){
  // Load up site only if it hasnt been passed in
  $args = drush_get_arguments();
  if (!is_array($site)) {
    $site = _kala_build_site($args[1]);
  }

  // Only attempt if this is a pantheon site
  if ($site['pantheon'] === TRUE) {
    drush_print('Coder, do you already have a local codebase for this site?');
    // The codebase might already exist
    if (is_dir("/var/www/" . $site['site'])) {
      if (drush_shell_exec("cat /var/www/" . $site['site'] . "/.git/config | grep url")) {
        drush_log('It looks like you might!', 'warning');
        drush_shell_exec("cat /var/www/" . $site['site'] . "/.git/config | grep url > /tmp/gitconfig" . $site['site']);
        $gitconfig_string = trim(file_get_contents("/tmp/gitconfig" . $site['site']));
        drush_shell_exec("rm /tmp/gitconfig" . $site['site']);
        // Codebase does exist
        if ($gitconfig_string == "url = ssh://codeserver.dev." . $site['id'] . "@codeserver.dev." . $site['id'] . ".drush.in:2222/~/repository.git") {
          drush_log('Oh you! You have already downloaded this code. I guess we will just refresh it then.', 'warning');
          drush_shell_exec("git --git-dir=/var/www/" . $site['site']. "/.git pull");
          drush_log('And... we refreshed that stale stale code!', 'success');
        }
        // A codebase exists but it is the wrong one
        else {
          drush_log('Directory /var/www/' . $site['site'] . ' exists... and is not empty... and is a codebase but not the right codebase. Please remove this directory before you attempt to download your codebase. Chump!', 'error');
        }
      }
      // Files alraedy in the folder that arent a git repo
      else {
        drush_log('Directory /var/www/' . $site['site'] . ' exists... and is not empty... and is not a codebase. Please remove this directory before you attempt to download your codebase. You vicious knave you!', 'error');
      }
    }
    // Git clone for the first time
    else {
      drush_print('Coder, your code fields are fertile and unblemished. Prepare to receive a potent load of code seed. Also this may take a few minutes so please be patient...');
      if (drush_shell_exec("git clone ssh://codeserver.dev." . $site['id'] . "@codeserver.dev." . $site['id'] . ".drush.in:2222/~/repository.git /var/www/" . $site['site'])) {
        drush_log('Your code fields bloom!', 'success');
      }
    }
  }
  else {
    return;
  }
}


/**
 * This creates a new DB and fills it with data from pantheon, or refreshs that data if it already exists
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 * @return only on failure
 */
function kala_data($site = NULL){
  // Load uo site only if it hasnt been passed in
  $args = drush_get_arguments();
  if (!is_array($site)) {
    $site = _kala_build_site($args[1]);
  }

  // Only try this if its a pantheon site
  if ($site['pantheon'] === TRUE) {
    // Create a new database if it doesnt already exist
    if (drush_shell_exec("mysql -uroot -ppassword -hlocalhost -e \"create database " . $site['kaladb'] . "\"")) {
      drush_log('Dearest friend, a new database called ' . $site['kaladb'] . ' hath been created', 'success');
    }
    else {
      drush_log('This database already exists! What are you trying to do!!!', 'notice');
    }

    // Making sure the pantheon DB server is awake
    drush_log('Making sure the beast is awake...', 'success');
    if (drush_shell_exec("drush @" . $site['site'] . ".dev status")) {
      drush_log('THE BEAST AWAKENS!', 'success');
    }

    // Downloading and importing the data
    drush_print('Dear sir! We are now attempting a database download, please standby...');
    // Trying SQL-SYNC-PIPE first
    if (drush_shell_exec("cd /var/www/" . $site['site'] . " && drush pipe @" . $site['site'] . ".dev default -y --disable=pantheon_apachesolr --ssh-options='" . $site['ssh'] . "'")) {
      drush_log('SOMETHING SOMETHING SOMETHING IMPORT COMPLETE', 'success');
    }
    // Using ghetto method if SSP fails
    else {
      drush_log('SQL-SYNC seemed to fail... trying to rock it old school instead', 'notice');

      if (drush_shell_exec("mysqldump -u" . $site['db_user'] . " -h" . $site['db_host'] ." -P" . $site['db_port'] ." -p" . $site['db_pass'] . " -CceKqQ --add-drop-table --add-locks --create-options --no-autocommit --single-transaction " . $site['db_db'] ." > /tmp/" . $site['id'] .".sql")) {
        drush_log('data downloaded from pantheon', 'success');
      }
      else {
        drush_log('could not get data from pantheon', 'error');
      }
      if (drush_shell_exec("mysql -uroot -ppassword -hlocalhost " . $site['kaladb'] . " < /tmp/" . $site['id'] .".sql")) {
        drush_log('your pantheon data has been imported to ' . $site['kaladb'], 'success');
      }
    }

  }
  else {
    return;
  }
}

/**
 * Downloads or refreshs pantheon files
 * @param  array or string $site an array of the sites info or the uparsed machine argument
 * @return if fails
 */
function kala_files($site){
  // Load up site byt only if not passed in
  $args = drush_get_arguments();
  if (!is_array($site)) {
    $site = _kala_build_site($args[1]);
  }

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
function kala_build() {

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
  else {
    // Check to see if directory already exists for some weird reason
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
function kala_destroy(){

  // Get and parse sitename
  $args = drush_get_arguments();
  $site = _kala_build_site($args[1]);

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
 * Put together meaningful information about the site about to be created/built
 * @param  [string] $sitename [machine name of the host]
 * @return if fails
 */
function _kala_build_site($sitename) {
    // Load all available aliases and remove suffix
  drush_sitealias_load_all();
  $aliases = drush_get_context('site-aliases');
  $siteal = explode('.', $sitename);
  $site = array();

  // Set site machinename and locald b name
  $site['site'] = $siteal[0];
  $site['kaladb'] = str_replace("-", "_", $site['site'] . "_kala");

  // Build some common elements
  if (isset($aliases['@' . $sitename])) {
    // Honestly don't know what this line is for?
    // $site = explode('.', $aliases['@' . $sitename]['uri']);
    $user = explode('.', $aliases['@' . $sitename]['remote-user']);

    // Weak check if alias is valid pantheon alias
    if ($user[0] != 'dev') {
      drush_log('WE ONLY BUILD FROM DEV YO', 'error');
      return FALSE;
    }
    elseif (!isset($aliases['@' . $sitename]['remote-host']) || !isset($aliases['@' . $sitename]['remote-user']) || !isset($aliases['@' . $sitename]['db-url'] )) {
      drush_log('Alias exists but is probably not a validly formatted pantheon alias... doing nothing', 'success');
      return FALSE;
    }
    else {
      // Load pantheon environmental vars
      $site['pantheon'] = TRUE;
      $site['env'] = $user[0];
      $site['id'] = $user[1];
      $site['ssh'] = $aliases['@' . $sitename]['ssh-options'] . ' -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no';
      $site['db_url'] = $aliases['@' . $sitename]['db-url'];
      $site['built-from'] = $sitename;

      // Get Human Readable Sitename
      drush_shell_exec("drush @" . $sitename . " vget site_name");
      $drush_site_name = drush_shell_exec_output();
      preg_match('/\"([^\"]+)\"/', end($drush_site_name), $matches);
      $site['site-name'] = $matches[1];

      // Split out database
      $regex = '/^mysql:\/\/([^:]+):([^@]+)@([^:]+):(\d+)\/(.+)$/';
      $matches = array();
      preg_match($regex, $site['db_url'], $matches);
      $site['db_user'] = $matches[1];
      $site['db_pass'] = $matches[2];
      $site['db_host'] = $matches[3];
      $site['db_port'] = $matches[4];
      $site['db_db'] = $matches[5];

      return $site;
    }
  }
  else {
    $site['env'] = 'kala';
    return $site;
  }
}
