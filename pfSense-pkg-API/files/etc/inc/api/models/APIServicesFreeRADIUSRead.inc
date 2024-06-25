<?php
//   Copyright 2023 Jared Hendrickson
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

require_once("api/framework/APIModel.inc");
require_once("api/framework/APIResponse.inc");

class APIServicesFreeRADIUSRead extends APIModel {
    # Create our method constructor
    public function __construct() {
        parent::__construct();
        $this->privileges = ["page-all", "page-freeradius"];
        $this->requires_auth = true;
        $this->packages = ["pfSense-pkg-freeradius3"];

        # CALL YOUR VALIDATOR METHODS HERE. This must be an array of function calls!
        $this->validated_data = [
            $this->validate_payload()
        ];
    }

    # Overrides our APIModel validate_payload method to validate data within our initial_data property
    public function validate_payload() {
        # Check if we have a test value in our payload, if so add the value to validated_data array
        if (array_key_exists("test", $this->initial_data)) {
            $this->validated_data["test"] = $this->initial_data["test"];
        } 
        # Otherwise, append a new error response to our error list (see Writing API responses for more info)
        else {
            $this->errors[] = APIResponse\get(1);
        }      
    }

    # Tell our API model what to do after successfully validating the client's request
    public function action(){
        $this->id = $this->get_next_id("test/item");    // Get the next available array index for the test item
        $this->set_config("test/item/{$this->id}", $this->validated_data);    // Set a new 'test' item in the config
        $this->write_config();       // Apply the configuration change
        return APIResponse\get(0, $this->validated_data);   // Return a success response containing our added data
    }
}
