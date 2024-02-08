<?php

namespace RESTAPI\Forms;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Form;

class SystemRESTAPIUpdatesForm extends Form {
    public string $url = '/system_restapi_updates.php';
    public string $model_class = 'RESTAPIVersion';
    public array $title_path = ['System', 'REST API', 'Updates'];
    public array $tabs = [
        ['Settings', false, '/system_restapi_settings.php'],
        ['Keys', false, '/system_restapi_key.php'],
        ['Updates', true, '/system_restapi_updates.php'],
        ['Documentation', false, '/openapi/'],
    ];
    public array $sections = [
        'Version' => [
            'fields' => ['current_version', 'latest_version', 'latest_version_release_date'],
        ],
        'Install a different version' => [
            'fields' => ['install_version'],
        ],
    ];
}