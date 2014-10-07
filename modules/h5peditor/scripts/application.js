var H5PEditor = H5PEditor || {};
var ns = H5PEditor;
(function($) {
  ns.init = function () {
    var h5peditor;
    var $upload = $('#edit-h5p-wrapper');
    var $editor = $('.h5p-editor');
    var $create = $editor.parent();//.hide();
    var $type = $('input[name="h5p_type"]');
    var $params = $('input[name="json_content"]');
    var $library = $('input[name="h5p_library"]');
    var library = $library.val();

    ns.$ = H5P.jQuery;
    ns.basePath = Drupal.settings.basePath +  Drupal.settings.h5peditor.modulePath + '/h5peditor/';
    ns.contentId = Drupal.settings.h5peditor.nodeVersionId;
    ns.fileIcon = Drupal.settings.h5peditor.fileIcon;
    ns.ajaxPath = Drupal.settings.h5peditor.ajaxPath;
    ns.filesPath = Drupal.settings.h5peditor.filesPath;
    ns.contentLanguage = ns.$('#edit-language').val();

    var $lang = ns.$('#edit-language');
    var setLang = function () {
      ns.contentLanguage = $lang.val();
    };
    $lang.change(setLang);
    setLang();


    // Semantics describing what copyright information can be stored for media.
    ns.copyrightSemantics = Drupal.settings.h5peditor.copyrightSemantics;

    // Required styles and scripts for the editor
    ns.assets = Drupal.settings.h5peditor.assets;

    // Required for assets
    ns.baseUrl = Drupal.settings.basePath;

    $type.change(function () {
      if ($type.filter(':checked').val() === 'upload') {
        $create.hide();
        $upload.show();
      }
      else {
        $upload.hide();
        if (h5peditor === undefined) {
          h5peditor = new ns.Editor(library, $params.val(), $editor[0]);
        }
        $create.show();
      }
    }).change();

    $('#node-form').submit(function () {
      if (h5peditor !== undefined) {
        var params = h5peditor.getParams();

        if (params === false) {
          // return false;
          /*
           * TODO: Give good feedback when validation fails. Currently it seems save and delete buttons
           * aren't working, but the user doesn't get any indication of why they aren't working.
           */
        }

        if (params !== undefined) {
          $library.val(h5peditor.getLibrary());
          $params.val(JSON.stringify(params));
        }
      }
    });
  };

  ns.getAjaxUrl = function (action, parameters) {
    var url = Drupal.settings.h5peditor.ajaxPath + action;

    if (parameters !== undefined) {
      for (var key in parameters) {
        url += '/' + parameters[key];
      }
    }

    return url;
  };

  $(document).ready(ns.init);
})(H5P.jQuery);
