<?php

namespace RESTAPI\Dispatchers;

use RESTAPI\Core\Dispatcher;

class FirewallApplyDispatcher extends Dispatcher {
    # Do not limit the number concurrent filter reloads that can run
    public int $max_concurrency = 0;

    /**
     * Reloads the filter services.
     */
    protected function _process(mixed ...$arguments): void {
        filter_configure_sync();
    }
}