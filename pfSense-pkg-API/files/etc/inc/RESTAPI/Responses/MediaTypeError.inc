<?php

namespace RESTAPI\Responses;

require_once("RESTAPI/autoloader.inc");

use RESTAPI\Core\Response;

/**
 * Exception class tha indicates the request media type is not supported. This exception generally corresponds with a
 * 415 Unsupported Media Type HTTP code.
 */
class MediaTypeError extends Response {
    public $code = 415;
    public string $help_text = "The client has requested a content-type that is not supported by the server.";

    public function __construct(string $message, string $response_id, array $data = []) {
        parent::__construct($message, $response_id, $data);
    }
}