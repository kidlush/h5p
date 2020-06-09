/* global Drupal */
(function ($) { // Avoid leaking variables

  /**
   * Add CSRF token to link
   *
   * @param {HTMLAnchorElement} button Link
   * @param {string} token Token
   */
  const addCSRFToken = function (button, token) {
    const form = document.createElement('form');
    form.action = button.href;
    form.method = 'POST';
    form.classList.add('h5p-hidden-form');

    const inputToken = document.createElement('input');
    inputToken.name = 'token';
    inputToken.type = 'hidden';
    inputToken.value = token;
    form.appendChild(inputToken);

    button.parentNode.appendChild(form);

    button.href = '#' + button.href.match(/([^\/])+$/i)[0];
    button.addEventListener('click', function (e)  {
      form.submit();
      e.preventDefault();
    });
  }

  /**
   * Add CSRF token to link
   *
   * @param {HTMLAnchorElement} button Link
   * @param {string} token Token
   */
  const addSharing = function (button, settings) {
    const originalHref = button.href;
    const urlData = button.href.match(/\/node\/(\d+)\/([^\/]+)$/i);
    const publishPath = settings.publishData.replace(encodeURIComponent(':id'), urlData[1]);
    button.href = '#' + urlData[2];
    button.addEventListener('click', function (e)  {
      $.get(publishPath, function (data) {
        data.data.publishURL = originalHref;
        data.data.container = document.getElementById('block-system-main');
        H5PHub.createSharingUI(data.data);
      });
      e.preventDefault();
    });
  }

  Drupal.behaviors.h5pContentHub = {
    attach: function (context, settings) {
      const buttons = context.getElementsByClassName('h5p-content-hub-button');
      for (let i = 0; i < buttons.length; i++) {
        if (buttons[i].classList.contains('processed')) {
          continue; // Skip
        }

        if (buttons[i].classList.contains('publish')) {
          addSharing(buttons[i], settings.h5pContentHub);
        }
        else {
          addCSRFToken(buttons[i], settings.h5pContentHub.token);
        }
      }
    }
  };

})(jQuery);
