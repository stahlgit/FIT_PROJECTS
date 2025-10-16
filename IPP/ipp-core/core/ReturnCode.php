<?php

/**
 * IPP - PHP Project Core
 * @author Radim Kocman
 * @author Zbyněk Křivka
 * ---
 * DO NOT MODIFY THIS FILE!
 */

namespace IPP\Core;

/**
 * Common script return codes from the project specification
 */
abstract class ReturnCode
{
    public const int OK = 0;
    public const int PARAMETER_ERROR = 10;
    public const int INPUT_FILE_ERROR = 11;
    public const int OUTPUT_FILE_ERROR = 12;
    public const int PARSE_LEXICAL_ERROR = 21;
    public const int PARSE_SYNTAX_ERROR = 22;
    public const int PARSE_MAIN_ERROR = 31;
    public const int PARSE_UNDEF_ERROR = 32;
    public const int PARSE_ARITY_ERROR = 33;
    public const int PARSE_COLLISION_ERROR = 34;
    public const int INVALID_XML_ERROR = 41;
    public const int INVALID_SOURCE_STRUCTURE_ERROR = 42;
    public const int INTERPRET_DNU_ERROR = 51;
    public const int INTERPRET_TYPE_ERROR = 52;
    public const int INTERPRET_VALUE_ERROR = 53;
    public const int INTEGRATION_ERROR = 88;
    public const int INTERNAL_ERROR = 99;
}
