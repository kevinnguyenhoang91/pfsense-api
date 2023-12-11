<?php

namespace RESTAPI\Forms;

require_once("RESTAPI/autoloader.inc");

use RESTAPI\Core\Form;

class SystemRESTAPIUpdates extends Form {
    public string $url = "/system_restapi_updates.php";
    public string $model_name = "\RESTAPI\Models\APIVersion";
    public array $title_path = ["System", "API", "Updates"];
    public string $custom_css = "/css/RESTAPI/RESTAPI.css";
    public array $tabs = [
        ["Settings", false, "/system_restapi_settings.php"],
        ["Keys", false, "/system_restapi_key.php"],
        ["Updates", true, "/system_restapi_updates.php"],
        ["Documentation", false, "/RESTAPI/documentation/"]
    ];
    public array $sections = [
        "Version" => [
            "Fields" => ["current_version", "latest_version", "latest_version_release_date"]
        ],
        "Install a different version" => [
            "Fields" => ["install_version"]
        ]
    ];
}