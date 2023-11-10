<?php

namespace api\tests;

require_once("api/auto_loader.inc");

use API\Core\TestCase;
use API\Models\APIKey;

class APIModelsAPIKeyTestCase extends TestCase
{
    /**
     * Checks that API keys can only be issued to existing users.
     */
    public function test_api_keys_can_only_be_issued_to_existing_users() {
        $this->assert_throws_response(
            response_id: "API_KEY_USERNAME_MUST_EXIST",
            code: 400,
            callable: function () {
                $key = new APIKey();
                $key->validate_username("not_an_existing_user");
            }
        );

        $this->assert_does_not_throw(
            callable: function () {
                $key = new APIKey();
                $key->validate_username("admin");
            }
        );
    }

    /**
     * Checks that the `key` and `hash` values are populated after API key creation.
     */
    public function test_key_and_hash_populated_after_creation() {
        # Construct an APIKey model object to test with
        $key = new APIKey(representation_data: ["name" => "test", "username" => "admin"]);

        # Ensure there is no `key` or `hash` value set before creation.
        $this->assert_is_empty($key->key->value);
        $this->assert_is_empty($key->hash->value);

        # Create the APIKey and ensure it now has `key` and `hash` values
        $key->create();
        $this->assert_is_not_empty($key->key->value);
        $this->assert_is_not_empty($key->hash->value);

        # Ensure `hash` is a hashed represention of the `key` value
        $this->assert_equals(
            $key->hash->value,
            hash($key->hash_algo->value, $key->key->value)
        );

        # Ensure the `key` value is not stored internally
        $this->assert_is_empty($key->to_internal()["key"]);

        # Ensure the `hash` value is not included in representation
        $this->assert_is_empty($key->to_representation()["hash"]);
        $key->delete();
    }

    /**
     * Checks that the `authenticate()` method correctly authenticates API keys.
     */
    public function test_authenticate() {
        # Construct an APIKey model object to test with
        $key = new APIKey(representation_data: ["name" => "test", "username" => "admin"]);
        $key->create();

        # Ensure we can authenticate using the actual `key`
        $this->assert_is_true($key->authenticate($key->key->value));

        # Ensure authentication fails when the `key` is incorrect
        $this->assert_is_false($key->authenticate("not our api key!"));
        $key->delete();
    }

    /**
     * Checks that APIKey model objects cannot be updated.
     */
    public function test_api_keys_cannot_be_updated() {
        $this->assert_throws_response(
            response_id: "API_KEY_CANNOT_BE_UPDATED",
            code: 500,
            callable: function () {
                $key = new APIKey(representation_data: ["name" => "test", "username" => "admin"]);
                $key->create();
                $key->update();
            }
        );
    }
}