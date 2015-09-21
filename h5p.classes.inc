<?php

// TODO: Validate file endings against a setting for security
class H5PDrupal implements H5PFrameworkInterface {
  /**
   * Implements setErrorMessage
   */
  public function setErrorMessage($message) {
    if (user_access('create h5p')) {
      drupal_set_message($message, 'error');
    }
  }

  /**
   * Implements setInfoMessage
   */
  public function setInfoMessage($message) {
    if (user_access('create h5p')) {
      drupal_set_message($message);
    }
  }

  /**
   * Implements t
   */
  public function t($message, $replacements = array()) {
    return t($message, $replacements);
  }

  /**
   * Implements getH5PPath
   */
  public function getH5pPath() {
    return _h5p_get_h5p_path();
  }

  /**
   * Implements getUploadedH5PFolderPath
   */
  public function getUploadedH5pFolderPath() {
    return $_SESSION['h5p_upload_folder'];
  }

  /**
   * Implements getUploadedH5PPath
   */
  public function getUploadedH5pPath() {
    return $_SESSION['h5p_upload'];
  }

  /**
   * Implements loadLibraries
   */
  public function loadLibraries() {
    $res = db_query('SELECT library_id as id, machine_name as name, title, major_version, minor_version, patch_version, runnable, restricted FROM {h5p_libraries} ORDER BY title ASC, major_version ASC, minor_version ASC');

    $libraries = array();
    while ($library = db_fetch_object($res)) {
      $libraries[$library->name][] = $library;
    }

    return $libraries;
  }

  /**
   * Implementation of getUnsupportedLibraries
   */
  public function getUnsupportedLibraries() {
    return variable_get('h5p_unsupported_libraries', NULL);
  }

  /**
   * Implementation of setUnsupportedLibraries
   */
  public function setUnsupportedLibraries($libraries) {
    variable_set('h5p_unsupported_libraries', $libraries);
  }

  /**
   * Implements getAdminUrl
   */
  public function getAdminUrl() {
    return url('admin/content/h5p/libraries');
  }

  /**
   * Implements getLibraryId
   */
  public function getLibraryId($machineName, $majorVersion, $minorVersion) {
    $library_id = db_result(db_query(
      "SELECT library_id
      FROM {h5p_libraries}
      WHERE machine_name = '%s'
      AND major_version = %d
      AND minor_version = %d",
      $machineName, $majorVersion, $minorVersion));
    return $library_id;
  }

  /**
   * Implements isPatchedLibrary
   */
  public function isPatchedLibrary($library) {
    $operator = $this->isInDevMode() ? '<=' : '<';
    $result = db_result(db_query(
      "SELECT 1
      FROM {h5p_libraries}
      WHERE machine_name = '%s'
      AND major_version = %d
      AND minor_version = %d
      AND patch_version $operator %d",
      $library['machineName'], $library['majorVersion'], $library['minorVersion'], $library['patchVersion']));
    return $result === '1';
  }

  /**
   * Implements isInDevMode
   */
  public function isInDevMode() {
    return (bool) variable_get('h5p_dev_mode', '0');
  }

  /**
   * Implements mayUpdateLibraries
   */
  public function mayUpdateLibraries() {
    return user_access('update h5p libraries');
  }

  /**
   * Implements getLibraryUsage
   *
   * Get number of content/nodes using a library, and the number of
   * dependencies to other libraries
   *
   * @param int $libraryId
   * @return array The array contains two elements, keyed by 'content' and 'libraries'.
   *               Each element contains a number
   */
  public function getLibraryUsage($libraryId, $skipContent = FALSE) {
    $usage = array();

    $usage['content'] = $skipContent ? -1 : intval(db_result(db_query(
      'SELECT COUNT(distinct n.nid)
      FROM {h5p_libraries} l JOIN {h5p_nodes_libraries} nl ON l.library_id = nl.library_id JOIN {h5p_nodes} n ON nl.content_id = n.content_id
      WHERE l.library_id = %d', $libraryId)));

    $usage['libraries'] = intval(db_result(db_query("SELECT COUNT(*) FROM {h5p_libraries_libraries} WHERE required_library_id = %d", $libraryId)));

    return $usage;
  }

  /**
   * Implements saveLibraryData
   */
  public function saveLibraryData(&$libraryData, $new = TRUE) {
    $preloadedJs = $this->pathsToCsv($libraryData, 'preloadedJs');
    $preloadedCss =  $this->pathsToCsv($libraryData, 'preloadedCss');
    $dropLibraryCss = '';

    if (isset($libraryData['dropLibraryCss'])) {
      $libs = array();
      foreach ($libraryData['dropLibraryCss'] as $lib) {
        $libs[] = $lib['machineName'];
      }
      $dropLibraryCss = implode(', ', $libs);
    }

    $embedTypes = '';
    if (isset($libraryData['embedTypes'])) {
      $embedTypes = implode(', ', $libraryData['embedTypes']);
    }
    if (!isset($libraryData['semantics'])) {
      $libraryData['semantics'] = '';
    }
    if (!isset($libraryData['fullscreen'])) {
      $libraryData['fullscreen'] = 0;
    }
    if ($new) {
      db_query("INSERT INTO {h5p_libraries}
        (machine_name, title, major_version, minor_version, patch_version, runnable, fullscreen, embed_types, preloaded_js, preloaded_css, drop_library_css, semantics)
        VALUES ('%s', '%s', %d, %d, %d, %d, %d, '%s', '%s', '%s', '%s', '%s')",
        $libraryData['machineName'], $libraryData['title'], $libraryData['majorVersion'], $libraryData['minorVersion'],
        $libraryData['patchVersion'], $libraryData['runnable'], $libraryData['fullscreen'], $embedTypes, $preloadedJs,
        $preloadedCss, $dropLibraryCss, $libraryData['semantics']
      );
      $libraryId = db_last_insert_id('h5p_libraries', 'library_id');
      $libraryData['libraryId'] = $libraryId;
      if ($libraryData['runnable']) {
        if (!variable_get('h5p_first_runnable_saved', 0)) {
          variable_set('h5p_first_runnable_saved', 1);
        }
      }
    }
    else {
      db_query("UPDATE {h5p_libraries}
        SET title = '%s', patch_version = %d, runnable = %d, fullscreen = %d,
        embed_types = '%s', preloaded_js = '%s', preloaded_css = '%s', drop_library_css = '%s', semantics = '%s'
        WHERE library_id = %d",
        $libraryData['title'], $libraryData['patchVersion'], $libraryData['runnable'], $libraryData['fullscreen'], $embedTypes,
        $preloadedJs, $preloadedCss, $dropLibraryCss, $libraryData['semantics'], $libraryData['libraryId']
      );
      $this->deleteLibraryDependencies($libraryData['libraryId']);
    }
    // Update languages
    db_query("DELETE FROM {h5p_libraries_languages}
      WHERE library_id = %d", $libraryData['libraryId']);
    if (isset($libraryData['language'])) {
      foreach ($libraryData['language'] as $languageCode => $languageJson) {
        db_query("INSERT INTO {h5p_libraries_languages}
          (library_id, language_code, language_json)
          VALUES (%d, '%s', '%s')", $libraryData['libraryId'], $languageCode, $languageJson);
      }
    }
  }

  /**
   * Convert list of file paths to csv
   *
   * @param array $libraryData
   *  Library data as found in library.json files
   * @param string $key
   *  Key that should be found in $libraryData
   * @return string
   *  file paths separated by ', '
   */
  private function pathsToCsv($libraryData, $key) {
    if (isset($libraryData[$key])) {
      $paths = array();
      foreach ($libraryData[$key] as $file) {
        $paths[] = $file['path'];
      }
      return implode(', ', $paths);
    }
    return '';
  }

  /**
   * Implements deleteLibraryDependencies
   */
  public function deleteLibraryDependencies($libraryId) {
    db_query("DELETE FROM {h5p_libraries_libraries} WHERE library_id = %d", $libraryId);
  }

  /**
   * Implements deleteLibrary. Will delete a library's data both in the database and file system
   */
  public function deleteLibrary($libraryId) {
    $library = db_fetch_object(db_query("select * from {h5p_libraries} where library_id = %d", $libraryId));

    // Delete files:
    H5PCore::deleteFileTree(file_directory_path() . '/' . variable_get('h5p_default_path', 'h5p') . '/libraries/' . $library->machine_name . '-' . $library->major_version . '.' . $library->minor_version);

    // Delete data in database (won't delete content):
    db_query("DELETE FROM {h5p_libraries_libraries} WHERE library_id = %d", $libraryId);
    db_query("DELETE FROM {h5p_libraries_languages} WHERE library_id = %d", $libraryId);
    db_query("DELETE FROM {h5p_libraries} WHERE library_id = %d", $libraryId);
  }

  /**
   * Implements saveLibraryDependencies
   */
  public function saveLibraryDependencies($libraryId, $dependencies, $dependency_type) {
    foreach ($dependencies as $dependency) {
      db_query(
        "INSERT INTO {h5p_libraries_libraries} (library_id, required_library_id, dependency_type)
        SELECT %d, hl.library_id, '%s'
        FROM {h5p_libraries} hl
        WHERE machine_name = '%s'
        AND major_version = %d
        AND minor_version = %d
        ON DUPLICATE KEY UPDATE dependency_type='%s'",
        $libraryId, $dependency_type, $dependency['machineName'], $dependency['majorVersion'], $dependency['minorVersion'], $dependency_type
      );
    }
  }

  /**
   * Implements updateContent
   */
  public function updateContent($content, $contentMainId = NULL) {
    $content_id = db_result(db_query("SELECT content_id FROM {h5p_nodes} WHERE content_id = %d", $content['id']));
    if ($content_id === FALSE) {
      // This can happen in Drupal when module is reinstalled. (since the ID is predetermined)
      $this->insertContent($content, $contentMainId);
      return;
    }

    db_query("UPDATE {h5p_nodes}
        SET json_content = '%s', embed_type = 'div', main_library_id = %d, filtered = '', disable = '%s'
        WHERE content_id = %d",
        $content['params'], $content['library']['libraryId'], $content['disable'], $content_id
    );

    // TODO: Use more specific code to remove just h5p-X-X.js/css ?
    // TODO: Use own cache busting string for each content ?
    _drupal_flush_css_js();
    drupal_clear_js_cache();
    drupal_clear_css_cache();
  }

  /**
   * Implements insertContent
   */
  public function insertContent($content, $contentMainId = NULL) {
    db_query(
        "INSERT INTO {h5p_nodes}
          (content_id, nid, json_content, embed_type, main_library_id, disable, filtered, slug)
          VALUES (%d, %d, '%s', '%s', %d, '%s', '', '')",
        $content['id'], $contentMainId, $content['params'], 'div', $content['library']['libraryId'], $content['disable']
    );
    // TODO: Add support for allowing the user to select embed type. Should be validated against what's possible for the library.

    // The identifier is returned because not all systems have a content id before the content is saved.
    return $content['id'];
  }

  /**
   * Implements resetContentUserData
   */
  public function resetContentUserData($contentId) {
    // Reset user data for this content
    db_query("UPDATE {h5p_content_user_data}
              SET timestamp = %d,
                  data = 'RESET'
              WHERE content_main_id = %d
              AND delete_on_content_change = 1",
             time(), $contentId);
  }

  /**
   * Implement getWhitelist
   */
  public function getWhitelist($isLibrary, $defaultContentWhitelist, $defaultLibraryWhitelist) {
    $whitelist = variable_get('h5p_whitelist', $defaultContentWhitelist);
    if ($isLibrary) {
      $whitelist .= ' ' . variable_get('h5p_library_whitelist_extras', $defaultLibraryWhitelist);
    }
    return $whitelist;
  }

  /**
   * Implements copyLibraryUsage
   */
  public function copyLibraryUsage($contentId, $copyFromId, $contentMainId = NULL) {
    db_query(
      "INSERT INTO {h5p_nodes_libraries} (content_id, library_id, dependency_type, drop_css, weight)
      SELECT %d, hnl.library_id, hnl.dependency_type, hnl.drop_css, hnl.weight
      FROM {h5p_nodes_libraries} hnl
      WHERE hnl.content_id = %d", $contentId, $copyFromId
    );
  }

  /**
   * Implements deleteContentData
   */
  public function deleteContentData($contentId) {
    db_query("DELETE FROM {h5p_nodes} WHERE content_id = %d", $contentId);
    $this->deleteLibraryUsage($contentId);
  }

  /**
   * Implements deleteLibraryUsage
   */
  public function deleteLibraryUsage($contentId) {
    db_query("DELETE FROM {h5p_nodes_libraries} WHERE content_id = %d", $contentId);
  }

  /**
   * Implements saveLibraryUsage
   *
   * TODO: Does the name saveContentDependencies() better describe what this function does?
   */
  public function saveLibraryUsage($contentId, $librariesInUse) {
    $dropLibraryCssList = array();

    foreach ($librariesInUse as $dependency) {
      if (!empty($dependency['library']['dropLibraryCss'])) {
        $dropLibraryCssList = array_merge($dropLibraryCssList, explode(', ', $dependency['library']['dropLibraryCss']));
      }
    }

    foreach ($librariesInUse as $dependency) {
      $dropCss = in_array($dependency['library']['machineName'], $dropLibraryCssList) ? 1 : 0;
      // If you get a duplicate error here there's something wrong with your data.
      // Remember that not all systems can do INSERT IGNORE or ON DUPLICATE KEY UPDATE.
      db_query(
          "INSERT INTO {h5p_nodes_libraries}
            (content_id, library_id, dependency_type, drop_css, weight)
            VALUES (%d, %d, '%s', %d, %d)",
          $contentId, $dependency['library']['libraryId'], $dependency['type'], $dropCss, $dependency['weight']
      );
    }
  }

  /**
   * Implements loadLibrary
   */
  public function loadLibrary($machineName, $majorVersion, $minorVersion) {
    $library = db_fetch_object(db_query(
        "SELECT library_id,
                machine_name,
                title,
                major_version,
                minor_version,
                patch_version,
                embed_types,
                preloaded_js,
                preloaded_css,
                drop_library_css,
                fullscreen,
                runnable
          FROM {h5p_libraries}
          WHERE machine_name = '%s'
          AND major_version = %d
          AND minor_version = %d",
      $machineName, $majorVersion, $minorVersion));

    if ($library === FALSE) {
      return FALSE;
    }
    $library = H5PCore::snakeToCamel($library);

    $result = db_query(
      "SELECT hl.machine_name as name,
              hl.major_version as major,
              hl.minor_version as minor,
              hll.dependency_type as type
      FROM {h5p_libraries_libraries} hll
      JOIN {h5p_libraries} hl ON hll.required_library_id = hl.library_id
      WHERE hll.library_id = %d", $library['libraryId']
    );
    while ($dependency = db_fetch_object($result)) {
      $library[$dependency->type . 'Dependencies'][] = array(
        'machineName' => $dependency->name,
        'majorVersion' => $dependency->major,
        'minorVersion' => $dependency->minor,
      );
    }

    return $library;
  }

  private function getSemanticsFromFile($machineName, $majorVersion, $minorVersion) {
    $semanticsPath = file_create_path(file_directory_path() . '/' . variable_get('h5p_default_path', 'h5p') . '/libraries/' . $machineName . '-' . $majorVersion . '.' . $minorVersion . '/semantics.json');
    if (file_exists($semanticsPath)) {
      $semantics = file_get_contents($semanticsPath);
      if (!json_decode($semantics, TRUE)) {
        drupal_set_message(t('Invalid json in semantics for %library', array('%library' => $machineName)), 'warning');
      }
      return $semantics;
    }
    return FALSE;
  }

  /**
   * Implements loadLibrarySemantics().
   */
  public function loadLibrarySemantics($machineName, $majorVersion, $minorVersion) {
    if ($this->isInDevMode()) {
      $semantics = $this->getSemanticsFromFile($machineName, $majorVersion, $minorVersion);
    }
    else {
      $semantics = db_result(db_query(
          "SELECT semantics
            FROM {h5p_libraries}
            WHERE machine_name = '%s'
            AND major_version = %d
            AND minor_version = %d",
        $machineName,
        $majorVersion,
        $minorVersion));
    }
    return ($semantics === FALSE ? NULL : $semantics);
  }

  /**
   * Implements alterLibrarySemantics().
   */
  public function alterLibrarySemantics(&$semantics, $name, $majorVersion, $minorVersion) {
    drupal_alter('h5p_semantics', $semantics, $name, $majorVersion, $minorVersion);
  }

  /**
   * Implements loadContent().
   */
  public function loadContent($id) {
    $content = db_fetch_object(db_query(
        "SELECT hn.content_id AS id,
                hn.json_content AS params,
                hn.embed_type,
                n.title,
                n.language,
                hl.library_id,
                hl.machine_name AS library_name,
                hl.major_version AS library_major_version,
                hl.minor_version AS library_minor_version,
                hl.embed_types AS library_embed_types,
                hl.fullscreen AS library_fullscreen,
                hn.filtered,
                hn.disable
          FROM {h5p_nodes} hn
          JOIN {node} n ON n.nid = hn.nid
          JOIN {h5p_libraries} hl ON hl.library_id = hn.main_library_id
          WHERE content_id = %d", $id));

    return ($content === FALSE ? NULL : H5PCore::snakeToCamel($content));
  }

  /**
   * Implements loadContentDependencies().
   */
  public function loadContentDependencies($id, $type = NULL) {
    $query =
        "SELECT hl.library_id,
                hl.machine_name,
                hl.major_version,
                hl.minor_version,
                hl.patch_version,
                hl.preloaded_css,
                hl.preloaded_js,
                hnl.drop_css,
                hnl.dependency_type
          FROM {h5p_nodes_libraries} hnl
          JOIN {h5p_libraries} hl ON hnl.library_id = hl.library_id
          WHERE hnl.content_id = %d";
    $queryArgs = array($id);

    if ($type !== NULL) {
      $query .= " AND hnl.dependency_type = '%s'";
      $queryArgs[] = $type;
    }
    $query .= " ORDER BY hnl.weight";
    $result = db_query($query, $queryArgs);

    $dependencies = array();
    while ($dependency = db_fetch_object($result)) {
      $dependencies[] = H5PCore::snakeToCamel($dependency);
    }

    return $dependencies;
  }

  /**
   * Implements getOption().
   */
  public function getOption($name, $default = NULL) {
    return variable_get('h5p_' . $name, $default);
  }


  /**
   * Implements setOption().
   */
  public function setOption($name, $value) {
    variable_set('h5p_' . $name, $value);
  }

  /**
   * Convert variables to fit our DB.
   */
  private static function camelToString($input) {
    $input = preg_replace('/[a-z0-9]([A-Z])[a-z0-9]/', '_$1', $input);
    return strtolower($input);
  }

  /**
   * Implements updateContentFields().
   */
  public function updateContentFields($id, $fields) {
    $set = '';
    $values = array();
    foreach ($fields as $name => $value) {
      if ($set !== '') {
        $set .= ', ';
      }
      $set .= self::camelToString($name) . ' = ' . (is_numeric($value) ? '%d' : "'%s'");
      $values[] = $value;
    }

    $values[] = $id;
    db_query("UPDATE {h5p_nodes}
      SET $set
      WHERE content_id = %d",
      $values
    );
  }

  /**
   * Implements clearFilteredParameters().
   */
  public function clearFilteredParameters($library_id) {
    db_query("UPDATE {h5p_nodes}
      SET filtered = ''
      WHERE main_library_id = %d",
      $library_id
    );

    // TODO: Use more specific code to remove just h5p-X-X.js/css ?
    // TODO: Use own cache busting string for each content ?
    _drupal_flush_css_js();
    drupal_clear_js_cache();
    drupal_clear_css_cache();
  }

  /**
   * Implements getNumNotFiltered().
   */
  public function getNumNotFiltered() {
    return (int) db_result(db_query("SELECT COUNT(content_id) FROM {h5p_nodes} WHERE filtered = '' AND main_library_id > 0"));
  }

  /**
   * Implements getNumContent().
   */
  public function getNumContent($library_id) {
    return (int) db_result(db_query('SELECT COUNT(content_id) FROM {h5p_nodes} WHERE main_library_id = %d', $library_id));
  }

  /**
   * Implements getPlatformInfo
   */
  public function getPlatformInfo() {
    // Could call system_get_info(), but performance-wise it
    // does not seem to be smart
    $h5p_info = unserialize(db_result(db_query("SELECT info FROM {system} WHERE name = 'h5p'")));

    return array(
      'name' => 'Drupal',
      'version' => VERSION,
      'h5pVersion' => isset($h5p_info->version) ? $h5p_info->version : NULL,
    );
  }

  /**
   * Implements fetchExternalData
   */
  public function fetchExternalData($url) {
    $response = drupal_http_request($url);
    return isset($response->error) ? NULL : $response->data;
  }

  /**
   * Implements setLibraryTutorialUrl
   */
  public function setLibraryTutorialUrl($machineName, $tutorialUrl) {
    db_query("UPDATE h5p_libraries SET tutorial_url='%s' WHERE machine_name = '%s'", $tutorial_url, $machineName);
  }

  // Only use if you have a feather in your hat!
  public function lockDependencyStorage() {}
  public function unlockDependencyStorage() {}

  /**
   * Implements isContentSlugAvailable
   */
  public function isContentSlugAvailable($slug) {
    return !db_result(db_query("SELECT slug FROM {h5p_nodes} WHERE slug = '%s'", $slug));
  }
}
