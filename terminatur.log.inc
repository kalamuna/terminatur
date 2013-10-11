<?php

 /**
 * @file
 *  log.inc: Overrides the drush log callback to not output to std error
 *  as this messes with Kalabox
 */

// Set our own log printing function:
drush_set_context('DRUSH_LOG_CALLBACK', 'terminatur_print_log');

/**
 * Override of _drush_print_log().
 * It's mostly the same except for having non-error mesages print to standard
 * out instead of standard error.
 */
function terminatur_print_log($entry) {
  if (drush_get_context('DRUSH_NOCOLOR')) {
    $red = "[%s]";
    $yellow = "[%s]";
    $green = "[%s]";
  }
  else {
    $red = "\033[31;40m\033[1m[%s]\033[0m";
    $yellow = "\033[1;33;40m\033[1m[%s]\033[0m";
    $green = "\033[1;32;40m\033[1m[%s]\033[0m";
  }

  $verbose = drush_get_context('DRUSH_VERBOSE');
  $debug = drush_get_context('DRUSH_DEBUG');

  $return = TRUE;
  $output_type = NULL; // Will default to standard out.
  switch ($entry['type']) {
    case 'warning':
    case 'cancel':
      $type_msg = sprintf($yellow, $entry['type']);
      break;
    case 'failed':
    case 'error':
      $type_msg = sprintf($red, $entry['type']);
      $return = FALSE;
      $output_type = STDERR;
      break;
    case 'ok':
    case 'completed':
    case 'success':
    case 'status':
      // In quiet mode, suppress progress messages
      if (drush_get_context('DRUSH_QUIET')) {
        return TRUE;
      }
      $type_msg = sprintf($green, $entry['type']);
      break;
    case 'notice':
    case 'message':
    case 'info':
      if (!$verbose) {
        // print nothing. exit cleanly.
        return TRUE;
      }
      $type_msg = sprintf("[%s]", $entry['type']);
      break;
    default :
      if (!$debug) {
        // print nothing. exit cleanly.
        return TRUE;
      }
      $type_msg = sprintf("[%s]", $entry['type']);
      break;
  }

  // When running in backend mode, log messages are not displayed, as they will
  // be returned in the JSON encoded associative array.
  if (drush_get_context('DRUSH_BACKEND')) {
    return $return;
  }

  $columns = drush_get_context('DRUSH_COLUMNS', 80);

  $width[1] = 11;
  // Append timer and memory values.
  if ($debug) {
    $timer = sprintf('[%s sec, %s]', round($entry['timestamp'] -DRUSH_REQUEST_TIME, 2), drush_format_size($entry['memory']));
    $entry['message'] = $entry['message'] . ' ' . $timer;
  }

  $width[0] = ($columns - 11);

  $format = sprintf("%%-%ds%%%ds", $width[0], $width[1]);

  // Place the status message right aligned with the top line of the error message.
  $message = wordwrap($entry['message'], $width[0]);
  $lines = explode("\n", $message);
  $lines[0] = sprintf($format, $lines[0], $type_msg);
  $message = implode("\n", $lines);
  drush_print($message, 0, $output_type);
  return $return;
}
