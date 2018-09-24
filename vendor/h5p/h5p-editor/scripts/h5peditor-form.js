/* global ns */
/**
 * Construct a form from library semantics.
 */
ns.Form = function () {
  var self = this;

  this.params = {};
  this.passReadies = false;
  this.commonFields = {};

  this.$form = ns.$('' +
    '<div class="h5peditor-form">' +
      '<div class="tree"></div>' +
      '<div class="common collapsed hidden">' +
        '<div class="fields">' +
          '<p class="desc">' +
            ns.t('core', 'commonFieldsDescription') +
          '</p>' +
        '</div>' +
      '</div>' +
    '</div>'
  );
  this.$common = this.$form.find('.common > .fields');
  this.library = Object.keys(ns.libraryLoaded)[0];

  this.enableMetadata = ns.enableMetadata(this.library);
  // Add overlay
  this.$form.append('<div class="overlay"></div>');

  // Inject a custom text field for the metadata title
  var metaDataTitleSemantics = [{
    'name' : 'mainTitle',
    'type' : 'text',
    'label' : ns.t('core', 'title'),
    'description': ns.t('core', 'usedForSearchingReportsAndCopyrightInformation'),
    'optional': false
  }];

  // Ensure it has validation functions
  ns.processSemanticsChunk(metaDataTitleSemantics, {}, this.$form.children('.tree'), this);

  self.mainTitleField = ns.findField('mainTitle', this);

  // Give title field an ID
  self.mainTitleField.$item.attr('id', 'metadata-title-main-label');
  self.mainTitleField.$input.attr('id', 'metadata-title-main');

  // Add the metadata button
  const metadataButton = ns.$('' +
    '<div class="h5p-metadata-button-wrapper">' +
      '<div class="h5p-metadata-button-tip"></div>' +
      '<div class="h5p-metadata-toggler">' + ns.t('core', 'metadata') + '</div>' +
    '</div>');

  /*
   * Temporarily needed for old content where wrapper will not be created by
   * the editor. Can be removed as soon as the new content types are considered
   * to be the default.
   */
  if (!this.enableMetadata) {
    const $wrapper = ns.$('<div/>', {'class': 'h5p-editor-flex-wrapper'});
    this.$form.find('label.h5peditor-label-wrapper').wrap($wrapper);
    // This fixes CSS overrides done by some old custom editors, but should not be in core
    switch (this.library.split(' ')[0]) {
      case 'H5P.InteractiveVideo':
      case 'H5P.DragQuestion':
      case 'H5P.ImageHotspotQuestion':
        this.$form.find('#metadata-title-main-label').first().css('padding', '20px 20px 0 20px');
        break;

      case 'H5P.CoursePresentation':
        this.$form.find('#metadata-title-main-label').first().css('padding-bottom', '1em');
        break;
    }
  }
  else {
    this.$form.find('.h5p-editor-flex-wrapper').append(metadataButton);
    this.$form.find('.h5p-metadata-toggler').click(function () {
      self.$form.find('.h5p-metadata-wrapper').first().toggleClass('h5p-open');
      self.$form.find('.overlay').toggle();
    });
  }

  // Add title expand/collapse button
  ns.$('<div/>', {
    'class': 'h5peditor-label',
    title: ns.t('core', 'expandCollapse'),
    role: 'button',
    tabIndex: 0,
    html: '<span class="icon"></span>' + ns.t('core', 'commonFields'),
    on: {
      click: function () {
        self.$common.parent().toggleClass('collapsed');
      },
      keypress: function (event) {
        if ((event.charCode || event.keyCode) === 32) {
          self.$common.parent().toggleClass('collapsed');
          event.preventDefault();
        }
      }
    },
    prependTo: this.$common.parent()
  });

  // Alternate background colors
  this.zebra = "odd";
};

/**
 * Replace the given element with our form.
 *
 * @param {jQuery} $element
 * @returns {undefined}
 */
ns.Form.prototype.replace = function ($element) {
  $element.replaceWith(this.$form);
  this.offset = this.$form.offset();
  // Prevent inputs and selects in an h5peditor form from submitting the main
  // framework form.
  this.$form.on('keydown', 'input,select', function (event) {
    if (event.keyCode === 13) {
      // Prevent enter key from submitting form.
      return false;
    }
  });
};

/**
 * Remove the current form.
 */
ns.Form.prototype.remove = function () {
  ns.removeChildren(this.metadataChildren);
  ns.removeChildren(this.children);
  this.$form.remove();
};

/**
 * Wrapper for processing the semantics.
 *
 * @param {Array} semantics
 * @param {Object} defaultParams
 * @returns {undefined}
 */
ns.Form.prototype.processSemantics = function (semantics, defaultParams, metadata) {
  this.metadata = (metadata ? metadata : defaultParams.metadata || {});

  this.$metadataForm = ns.metadataForm(semantics, this.metadata, this.$form.children('.tree'), this);
  const metadataTitle = ns.findField('title', this);

  // Sync title fields of this editor form and a metadata form
  ns.sync(
    this.mainTitleField.$input,
    metadataTitle.$input
  );

  // Set the title
  const title = (this.metadata && this.metadata.title) ? this.metadata.title : '';
  this.mainTitleField.$input.val(title);

  // Overriding this.params with {} will lead to old content not being editable for now
  this.params = (defaultParams.params ? defaultParams.params : defaultParams);

  // Put metadata children in a separate array
  this.metadataChildren = this.children;

  // Create real children
  ns.processSemanticsChunk(semantics, this.params, this.$form.children('.tree'), this);
};

/**
 * Collect functions to execute once the tree is complete.
 *
 * @param {function} ready
 * @returns {undefined}
 */
ns.Form.prototype.ready = function (ready) {
  this.readies.push(ready);
};