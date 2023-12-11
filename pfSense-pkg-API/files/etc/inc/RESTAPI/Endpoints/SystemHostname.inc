<?php

namespace RESTAPI\Endpoints;

require_once("RESTAPI/autoloader.inc");

use RESTAPI\Core\Endpoint;

class SystemHostname extends Endpoint
{
    public function __construct() {
        # Set Endpoint attributes
        $this->url = "/RESTAPI/v2/system/hostname";
        $this->model_name = "SystemHostname";
        $this->request_method_options = ["GET", "PATCH"];
        $this->get_help_text = "Reads the current system hostname.";
        $this->patch_help_text = "Updates the system hostname.";

        # Construct the parent Endpoint object
        parent::__construct();
    }
}