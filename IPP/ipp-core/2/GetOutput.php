<?php

namespace IPP\Student;

use IPP\Core\Interface\OutputWriter;

final class GetOutput
{
    private static ?OutputWriter $stdout = null;

    public static function setStdout(OutputWriter $writer): void
    {
        self::$stdout = $writer;
    }
    public static function getStdout(): ?OutputWriter
    {
        return self::$stdout;
    }
}
