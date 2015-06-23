<?php

/**
 * Handles all communication with the database.
 */
class H5peditorDrupalStorage implements H5peditorStorage {

  /**
   * Empty contructor.
   */
  function __construct() { }

  /**
   * TODO
   */
  public function getLanguage($machineName, $majorVersion, $minorVersion, $language) {
    $lang = db_result(db_query(
      "SELECT language_json
      FROM {h5p_libraries_languages} hlt
      JOIN {h5p_libraries} hl ON hl.library_id = hlt.library_id
      WHERE hl.machine_name = '%s'
      AND hl.major_version = %d
      AND hl.minor_version = %d
      AND hlt.language_code = '%s'",
      $machineName, $majorVersion, $minorVersion, $language));

    return ($lang === FALSE ? NULL : $lang);
  }

  /**
   * TODO
   */
  public function addTmpFile($file) {
    global $user;

    db_query("INSERT INTO {files} (uid, filename, filepath, filemime, filesize, status, timestamp) VALUES (%d, '%s', '%s', '%s', %d, %d, %d)", $user->uid, $file->name, $file->path, $file->mime, $file->size, FILE_STATUS_TEMPORARY, time());
  }

  /**
   * TODO
   */
  public function keepFile($oldPath, $newPath) {
    db_query("UPDATE {files} SET filepath = '%s', status = %d WHERE filepath = '%s'", $newPath, FILE_STATUS_PERMANENT, $oldPath);
  }

  /**
   * TODO
   */
  public function removeFile($path) {
    db_query("DELETE FROM {files} WHERE filepath = '%s'", $path);
  }

  /**
   * TODO
   */
  public function getLibraries($libraries = NULL) {
    $super_user = user_access('create restricted h5p content types');

    if ($libraries !== NULL) {
      // Get details for the specified libraries only.
      $librariesWithDetails = array();
      foreach ($libraries as $library) {
        $details = db_fetch_object(db_query(
          "SELECT title, runnable, restricted
           FROM {h5p_libraries}
           WHERE machine_name = '%s'
           AND major_version = %d
           AND minor_version = %d
           AND semantics IS NOT NULL",
         $library->name,
         $library->majorVersion,
         $library->minorVersion));
        if ($details !== FALSE) {
          $library->title = $details->title;
          $library->runnable = $details->runnable;
          $library->restricted = $super_user ? FALSE : ($details->restricted === '1' ? TRUE : FALSE);
          $librariesWithDetails[] = $library;
        }
      }

      return $librariesWithDetails;
    }

    $libraries = array();

    $libraries_result = db_query(
      "SELECT machine_name AS name,
              title,
              major_version,
              minor_version,
              restricted
       FROM {h5p_libraries}
       WHERE runnable = 1
       AND semantics IS NOT NULL
       ORDER BY title");
    while ($library = db_fetch_object($libraries_result)) {
      // Convert result object properties to camelCase.
      $library = H5PCore::snakeToCamel($library, true);

      // Make sure we only display the newest version of a library.
      foreach ($libraries as $existingLibrary) {
        if ($library->name === $existingLibrary->name) {

          // Mark old ones
          // This is the newest
          if (($library->majorVersion === $existingLibrary->majorVersion && $library->minorVersion > $existingLibrary->minorVersion) ||
              ($library->majorVersion > $existingLibrary->majorVersion)) {
            $existingLibrary->isOld = TRUE;
          }
          else {
            $library->isOld = TRUE;
          }
        }
      }

      $library->restricted = $super_user ? FALSE : ($library->restricted === '1' ? TRUE : FALSE);

      // Add new library
      $libraries[] = $library;
    }

    return $libraries;
  }

  public function alterLibraryFiles(&$files, $libraries) {
    $mode = 'editor';
    $library_list = _h5p_dependencies_to_library_list($libraries);
    drupal_alter('h5p_scripts', $files['scripts'], $library_list, $mode);
    drupal_alter('h5p_styles', $files['styles'], $library_list, $mode);
  }

}
