# Terminatur

## Deprecated.

Many components of this software have reached end of life and the project is no longer actively maintained

Please check out the successor project [Kalabox2](https://github.com/kalabox/kalabox). This project is still under development but it very actively supported and should reach feature parity with Kalabox 1 shortly. 

Terminatur is a set of drush commands to extend the functionality provided by the amazing
[Terminus](https://github.com/pantheon-systems/terminus)

Specifically Terminatur allows you to build Pantheon sites on your local development environment.
It takes care of handling your settings, vhosts, aliases... all the stuff that is generally annoying.
You can also easily spin up new sites.

Terminatur was written with the underlying [Kalastack](https://github.com/kalamuna/kalastack) environment
in mind. However, Terminatur features a pluggable architecture and it should be easy to add support for
MAMP, Megladon or any NIX based development stack.

If you are interested in providing some of these environmental plugins please contact us!

## Requirements

* Drush 5.1+ - https://github.com/drush-ops/drush
* PHP 5.3.3+ with cURL
* Terminus - https://github.com/pantheon-systems/terminus
* (Optional) MySQL 5+ - But you can't really do much without it

## Installing Terminatur with Composer and Packagist

[Composer](http://getcomposer.org) is a dependency manager for PHP, and
[Packagist](https://packagist.org/) is the main Composer repository. Terminatur
can be found on Packagist as [kalamuna/terminatur](https://packagist.org/packages/kalamuna/terminatur)

The easiest way to install Composer for *nix (including Mac):

    curl -sS https://getcomposer.org/installer | php
    mv composer.phar /usr/local/bin/composer

More detailed installation instructions for multiple platforms can be found in
the [Composer Documentation](http://getcomposer.org/doc/00-intro.md).

### Normal installation

    # Download Terminatur for non-development use.
    composer create-project kalamuna/terminatur $HOME/.drush/terminatur -s dev --no-dev -n
    # Clear Drush's cache.
    drush cc drush

To get updates when available just run

    # Updating Terminatur.
    composer update --no-dev --working-dir $HOME/.drush/terminatur

## Installing Terminatur with Git

If you are unable to use Composer, Terminatur can be installed using git. This
method is not recommended as dependencies will not be automatically installed if
they're missing.

    git clone https://github.com/kalamuna/terminatur.git $HOME/.drush/terminatur
    # Clear Drush's cache.
    drush cc drush

## Quickstart

    # Authenticate with Pantheon and pull down your aliases
    drush ta
    # List terminatur site aliases
    drush sa | grep terminatur
    # Download a site with all its files
    drush pullsite mysite.dev # mysite.dev is from the alias @terminatur.mysite.dev
    # Remove this site
    drush crush mysite.dev

## Some more indepth things

- mysite.dev corresponds to @terminatur.mysite.dev
- mysite.local correspondes to @local.mysite.local
- mysite.kala correspondes to @kalastack.mysite.local

```bash
$ drush pullsite mysite.dev # Build a Pantheon site locally
$ drush pullsite mysite.dev --files # Build a Pantheon site locally and download your files
```
This will also add an entry to your hosts file, set up a vhost on supported environments and
add a local drush alias for you to work with. Your settings.php will be edited to contain
the local connection configuration. If you run this command a second time it will
refresh your code, data and files.
```bash
$ drush pullcode mysite.dev # Pulls down your Pantheon code with either git or wget
$ drush pulldata mysite.dev # Imports your Pantheon database from a specific backup, the latest backup, or a newly created backup
$ drush pullfiles mysite.dev # Pulls down your Pantheon files, either by rsync or wget
```
Running any of these commands more than once will simply fetch what's new and pull it down.
```bash
$ drush crush mysite.dev # Completely removes your local site
$ drush crush mysite.kala # Completely removes your local site
$ drush crush mysite.local # Completely removes your local site
```
Not much else required to explain here!
```bash
$ drush newsite mysite # Spins up a new local site
$ drush newsite mysite --profile=openatrium # Spins up a new local OpenAtrium site
```
Builds a new site and sets up hosts, vhosts, settings, aliases.
```bash
$ drush ta # Refreshes your Pantheon alias file and parses it for use with Terminatur
```
You should run this anytime you add/remove a site on Pantheon or make a backup.

Plus run drush newsite --help for a complete listing of command options.

### UPDATING YOUR HOST HOSTS FILE

Remember that if you have Terminatur installed in a VM you may have to edit your
host machine's host file to see your site in your browser.

## The Future

- Support for multidev
- Deploy new sites to Pantheon
- Support for MAMP, DAMP, Megladon, Proviso

## Contributing

Please use the [issue tracker](https://github.com/kalamuna/terminatur/issues) if you find any bugs or wish to contribute.

-------------------------------------------------------------------------------------
(C) 2013 Kalamuna LLC
