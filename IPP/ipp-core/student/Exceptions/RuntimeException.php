<?php

namespace IPP\Student\Exceptions;

use IPP\Core\Exception\IPPException;
use Throwable;

class RuntimeException extends IPPException
{
    public function __construct(string $message, int $code, Throwable|null $previous = null)
    {
        parent::__construct($message, $code, $previous, false);
    }
}
