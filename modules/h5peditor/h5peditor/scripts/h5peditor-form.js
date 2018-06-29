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
      '<div class="tree">' +
        '<div class="overlay"></div>' +
      '</div>' +
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
  this.library = '';

  // Inject a custom text field for the metadata title
  var metaDataTitleSemantics = [{
    'name' : 'title',
    'type' : 'text',
    'label' : ns.t('core', 'title'),
    'description': ns.t('core', 'usedForSearchingReportsAndCopyrightInformation'),
    'optional': false
  }];

  // Ensure it has validation functions
  ns.processSemanticsChunk(metaDataTitleSemantics, {}, this.$form.children('.tree'), this)

  // Give title field an ID
  this.$form.find('.field-name-title').attr('id', 'metadata-title-main-label');
  this.$form.find('.h5peditor-text').attr('id', 'metadata-title-main');

  // Add the metadata button
  // const metadataButton = ns.$('' +
  //   '<div class="h5p-metadata-button-wrapper">' +
  //     '<div class="h5p-metadata-button-tip"></div>' +
  //     '<div class="toggle-metadata">' + ns.t('core', 'metadata') + '</div>' +
  //   '</div>');

  //this.$form.find('.h5p-editor-flex-wrapper').append(metadataButton);


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
  const that = this;
  this.metadata = (metadata ? metadata : defaultParams.metadata || {});

  const $metadataForm = ns.metadataForm(semantics, this.metadata, this.$form.children('.tree'), this);

  // Sync title fields of this editor form and a metadata form
  ns.sync(
    this.$form.find('#metadata-title-main'),
    $metadataForm.find('.field-name-title').find('input')
  );

  // Set the title
  const title = (this.metadata && this.metadata.title) ? this.metadata.title : '';
  this.$form.find('input#metadata-title-main').val(title);

  const $metadataButton = ns.$('<button class="h5peditor-new-meta">' + ns.t('core', 'metadata') + '</button>');
  const copyPasteWrapper = this.$form.siblings('label.h5peditor-copypaste-wrap');
  //const copyPasteWrapper = this.$form.parent().find('.h5peditor-copypaste-wrap').first();
  if (copyPasteWrapper.length > 0) {
    copyPasteWrapper.prepend($metadataButton);
  }
  $metadataButton.click(function () {
    that.$form.find('.h5p-metadata-wrapper').first().toggleClass('h5p-open');
    that.$form.find('.overlay').toggle();
  });

  // Overriding this.params with {} will lead to old content not being editable for now
  this.params = (defaultParams.params ? defaultParams.params : defaultParams);
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
