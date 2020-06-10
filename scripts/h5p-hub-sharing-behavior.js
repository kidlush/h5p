Drupal.behaviors.h5pContentHubPublish = {
  attach: function (context, settings) {
    const publish = document.getElementById('h5p-publish');
    if (!publish.classList.contains('processed')) {
      publish.classList.add('processed');
      settings.h5pContentHubPublish.container = publish; // Not ideal :-)
      H5PHub.createSharingUI(settings.h5pContentHubPublish);
    }
  }
};
