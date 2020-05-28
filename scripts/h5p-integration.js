/* global Drupal */

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

  button.href = "#";
  button.addEventListener('click', function (e)  {
    form.submit();
    e.preventDefault();
  });
}

/**
 * Start the integration script
 */
const init = function () {
  const settings = Drupal.settings.h5pIntegration;

  const unpublishButton = document.querySelector('.h5p-content-hub-button.unpublish');
  if (unpublishButton) {
    addCSRFToken(unpublishButton, settings.unpublishToken);
  }

  const syncButton = document.querySelector('.h5p-content-hub-button.sync');
  if (syncButton) {
    addCSRFToken(syncButton, settings.syncToken);
  }
};

if (document.readyState === 'complete') {
  init();
}
else {
  document.onreadystatechange = function () {
    if (document.readyState === 'complete') {
      init();
    }
  }
}
