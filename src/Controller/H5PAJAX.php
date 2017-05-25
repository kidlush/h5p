<?php

namespace Drupal\h5p\Controller;

use Drupal\Core\Url;
use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\Database\Connection;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\HttpFoundation\JsonResponse;
use Drupal\h5p\H5PDrupal\H5PDrupal;

class H5PAJAX extends ControllerBase {

  protected $database;

  /**
   * DBExample constructor.
   */
  public function __construct(Connection $database) {
    $this->database = $database;
  }

  public static function create(ContainerInterface $container) {

    $controller = new static(
      $container->get('database')
    );
    return $controller;
  }

  /**
   * Access callback for the setFinished feature
   */
  function setFinished() {
    $id = filter_input(INPUT_POST, 'contentId', FILTER_VALIDATE_INT);
    $response = [$id];
    return new JsonResponse($response);
  }

  /**
   * Handles insert, updating and deleteing content user data through AJAX.
   *
   * @param string $content_id
   * @param string $data_id
   * @param string $sub_coontent_id
   * @return string JSON
   */
  function contentUserData($content_main_id, $data_id, $sub_content_id) {

    $user = \Drupal::currentUser();

    $response = (object) array(
      'success' => TRUE
    );

    $data = filter_input(INPUT_POST, 'data');
    $preload = filter_input(INPUT_POST, 'preload');
    $invalidate = filter_input(INPUT_POST, 'invalidate');
    if ($data !== NULL && $preload !== NULL && $invalidate !== NULL) {
      if (! \H5PCore::validToken('contentuserdata', filter_input(INPUT_GET, 'token'))) {
        $response->success = FALSE;
        $response->message = t('Invalid security token.');
        return new JsonResponse($response);
      }

      if ($data === '0') {
        // Remove data
        db_delete('h5p_content_user_data')
          ->condition('content_main_id', $content_main_id)
          ->condition('data_id', $data_id)
          ->condition('user_id', $user->id())
          ->condition('sub_content_id', $sub_content_id)
          ->execute();
      } else {
        // Wash values to ensure 0 or 1.
        $preload = ($preload === '0' ? 0 : 1);
        $invalidate = ($invalidate === '0' ? 0 : 1);

        // Determine if we should update or insert
        $update = db_query("SELECT content_main_id
                                   FROM {h5p_content_user_data}
                                   WHERE content_main_id = :content_main_id
                                     AND user_id = :user_id
                                     AND data_id = :data_id
                                     AND sub_content_id = :sub_content_id",
          array(
            ':content_main_id' => $content_main_id,
            ':user_id' => $user->id(),
            ':data_id' => $data_id,
            ':sub_content_id' => $sub_content_id,
          ))->fetchField();

        if ($update === FALSE) {
          // Insert new data
          db_insert('h5p_content_user_data')
            ->fields(array(
              'user_id' => $user->id(),
              'content_main_id' => $content_main_id,
              'sub_content_id' => $sub_content_id,
              'data_id' => $data_id,
              'timestamp' => time(),
              'data' => $data,
              'preloaded' => $preload,
              'delete_on_content_change' => $invalidate
            ))
            ->execute();
        }
        else {
          // Update old data
          db_update('h5p_content_user_data')
            ->fields(array(
              'timestamp' => time(),
              'data' => $data,
              'preloaded' => $preload,
              'delete_on_content_change' => $invalidate
            ))
            ->condition('user_id', $user->id())
            ->condition('content_main_id', $content_main_id)
            ->condition('data_id', $data_id)
            ->condition('sub_content_id', $sub_content_id)
            ->execute();
        }
      }

      \Drupal\Core\Cache\Cache::invalidateTags(['h5p_content:' . $content_main_id]);
      return new JsonResponse($response);
    } else {
      // Fetch data
      $response->data = db_query("SELECT data FROM {h5p_content_user_data}
                                WHERE user_id = :user_id
                                  AND content_main_id = :content_main_id
                                  AND data_id = :data_id
                                  AND sub_content_id = :sub_content_id",
        array(
          ':user_id' => $user->id(),
          ':content_main_id' => $content_main_id,
          ':sub_content_id' => $sub_content_id,
          ':data_id' => $data_id,
        ))->fetchField();
    }

    return new JsonResponse($response);
  }

}