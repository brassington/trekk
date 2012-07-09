<?php
/**
 * Processes HTML--cleans it and updates hrefs and img tokens
 *
 * Depends on QueryPath
 *
 */

class HTMLProcessor {
	
	protected $qp; // QueryPath
  
  protected $migration;

  /**
   * constructor
   *
   * set migration and load QueryPath
   */
  public function __construct($migration, $html = NULL) {
  	$this->migration = $migration;

    $directory = variable_get('querypath_directory', 'sites/all/libraries/QueryPath');
    require_once $directory . '/QueryPath.php';
    $this->qp = htmlqp($html);
  }

  /**
   * Update Media and clean HTML via check_markup().
   */
  public function execute() {
    $this->updateMediaTokens();
    
    $html = $this->qp->top('body')->innerHTML();
    return check_markup($html, 'purified_html'); // load 'purified HTML' format filter and run it against the text
  }
  
  /**
   * Update media tokens in anchor and img tags
   *
   * Replace with file_create_url or json_encoded array respectively.
   */
  public function updateMediaTokens() {
    $migration = $this->migration;
    $migration_base_name = $migration->getBaseName(); // again w/ the machine name 
    
    $qp = $this->qp;

    // Replace media tokens in anchor tags (ie, downloadable files--pdfs, docs, xls, etc)
    foreach ($qp->top()->find('a') as $a) {
      $href = $a->attr('href');
      $id = trim(array_pop(explode(':', $href)), ']');
      $key = trim(array_shift(explode(':', $href)), '[');

      $item = $migration->getMappedEntity(array($migration_base_name . '_media'), array(array('id' => $id)));

      if ($item) {
        $a->attr('href', file_create_url($item->uri));
      }
    }

    // Replace media tokens in img tags
    foreach ($qp->top()->find('img') as $img) {
      $src = $img->attr('src');
      $id = trim(array_pop(explode(':', $src)), ']');
      $key = trim(array_shift(explode(':', $src)), '[');
      $item = $migration->getMappedEntity(array($migration_base_name . '_media'), array(array('id' => $id)));

      if ($item) {
        // Filter syntax is just a json_encode()ed array
        $repl = json_encode(array(
          'type'       => 'media',
          'view_mode'  => 'media_large',
          'fid'        => (int)$item->fid,
          'attributes' => array(
            'alt'      => check_plain($img->attr('alt')),
            'title'    => check_plain($img->attr('title')),
            'height'   => (int)$img->attr('height'),
            'width'    => (int)$img->attr('width'),
          ),
        ));

        $img->replaceWith('[[' . $repl . ']]');
      }
    }
  }

}