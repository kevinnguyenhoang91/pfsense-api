<?php

namespace API\Core;

class Field
{
    public mixed $value = null;
    public array $types = [];
    public array $validators = [];
    public bool $many = false;
    public string $delimiter = ",";
    public array $choices = [];
    public bool $allow_empty = false;
    public bool $allow_null = false;
    public bool $required = false;
    public mixed $default = null;
    public bool $read_only = false;
    public string $description = "";
    public string $name = "";
    public string $internal_name = "";
    public array $when = [];
    protected object $context;

    /**
     * @param array $types An array of the types supported by this field. Each item must be a string representation of
     * the type as returned by PHP's gettype() function.
     * @param array $validators An array of API\Core\Validator objects to use when validating this field.
     * @param array $choices An array of explicitly choices this field's value can be.
     * @param bool $allow_empty Set to `true` if empty values (blank strings, 0, etc) should be allowed.
     * @param bool $allow_null Set to `true` if `null` values should be supported for this field. When `null` values
     * are supported, the underlying pfSense configuration field will be removed entirely when receiving a null value.
     * @param bool $required Set to `true` if this field is required to be explicitly set. $required and $default are
     * mutually exclusive.
     * @param mixed|null $default The default value to assign this field if no value was found. If not set, it assumed
     * this Field requires a value and should fail validation if no value was found.
     * @param bool $read_only Set to `true` to make this field read-only. Attempts to write to a read-only field will
     * result in a thrown exception.
     * @param bool $many Set to `true` if this field should accept an array of many values. `false` if only one value
     * should be allowed. If `true`, validation will run against each value in the value array individual.
     * @param string $delimiter Assigns the string delimiter to use when writing array values to config. This is only
     * available if $many is set to `true`. Defaults to `,` to store as comma-separated string.
     * @param string $description Sets a description for this field. This description will be used as the description
     * for this field in the API documentation.
     * @param string $internal_name The name of the field as it is stored in the pfSense configuration. If not set, the
     * internal name will utilize the $name of the Field object.
     */
    public function __construct(
        array $types = [],
        array $validators = [],
        array $choices = [],
        bool $allow_empty = false,
        bool $allow_null = false,
        bool $required = false,
        mixed $default = null,
        bool $read_only = false,
        bool $many = false,
        string $delimiter = ",",
        string $description = "",
        string $internal_name = "",
    ){
        # Assign values to object properties
        $this->types = $types;
        $this->validators = $validators;
        $this->choices = $choices;
        $this->allow_empty = $allow_empty;
        $this->allow_null = ($read_only) ? true : $allow_null;
        $this->required = $required;
        $this->default = $default;
        $this->read_only = $read_only;
        $this->many = $many;
        $this->delimiter = $delimiter;
        $this->description = $description;

        # Add NULL to our allowed types if `allow_null` or `read_only` is enabled
        if ($allow_null or $read_only) {
            $this->types[] = "NULL";
        }

        # Ensure the $default value is in $choices if set
        if ($default and $choices and !in_array($default, $choices, true)) {
            throw new ServerError(
                message: "Field object `default` must be one of available `choices`.",
                return: "INVALID_CHOICE_DEFAULT"
            );
        }

        # Only assign the internal_name if it is not empty.
        if ($internal_name) {
            $this->internal_name = $internal_name;
        }
    }


    /**
     * Runs all validations for this field.
     * @return bool `true` if all validation succeeded.
     * @throws ValidationError If any validations failed.
     * @throws ServerError If a programming error is detected.
     */
    public function validate() : bool {
        # Ensure this field object has a name set
        $this->check_field_names();

        # Ensure a value is specified if field is `required`
        $this->check_field_required();

        # Ensure a value is not set if field is `read_only`
        $this->check_field_read_only();

        # Ensure the value(s) to validate are always in an array, so we can loop.
        $values = $this->get_value_as_array();

        # Loop through each value to be validated
        foreach($values as $value) {
            # Ensure the value's type is supported
            $this->check_value_type($value);

            # Ensure this value is an available choice (if `choices` are defined for this field)
            $this->check_value_choice($value);

            # Ensure this value is not empty unless empty values are allowed by this field
            $this->check_value_empty($value);

            # Ensure this value is not null unless null values are allowed by this field
            $this->check_value_null($value);

            # Loop through each validator and run it's `validate` method against the current value.
            foreach ($this->validators as $validator) {
                $validator->validate($value);
            }
        }
        return true;
    }

    /**
     * When the field is required, checks if a value is set and checks if mutually exclusive options are both set.
     * @throws ServerError When `required` and `default` are both set OR `required` and `allow_null` are both set.
     * @throws ValidationError If `required` is set but no value was assigned.
     */
    private function check_field_required() {
        # Throw an error if `required` and `default` are both set
        if ($this->required and isset($this->default)) {
            throw new ServerError(
                message: "Required field `$this->name` cannot have a default.",
                return: "CANNOT_REQUIRE_DEFAULT"
            );
        }

        # Throw an error if `required` and `allow_null` are both set
        if ($this->required and $this->allow_null) {
            throw new ServerError(
                message: "Required field `$this->name` cannot allow null values.",
                return: "CANNOT_REQUIRE_NULL"
            );
        }

        # Throw an error if this field is required but no value was set.
        if ($this->required and is_null($this->value)) {
            throw new ValidationError(
                message: "Field `$this->name` is required.",
                return: "MISSING_REQUIRED_FIELD"
            );
        }
    }

    /**
     * Ensures read-only values do not have a value, default or choices.
     * @throws ServerError When `read_only` is set and `choices` or `default` are also set.
     * @throws ValidationError When `read_only` is set and a non-null value was requested.
     */
    private function check_field_read_only() {
        # Throw an error if the field is read only, but a default value was set
        if ($this->read_only and isset($this->default)) {
            throw new ServerError(
                message: "Read-only field `$this->name` cannot have a default value.",
                return: "NO_DEFAULT_WITH_READ_ONLY"
            );
        }

        # Throw an error if the field is read only, but choices were defined
        if ($this->read_only and $this->choices) {
            throw new ServerError(
                message: "Read-only field `$this->name` cannot have value choices.",
                return: "NO_CHOICES_WITH_READ_ONLY"
            );
        }

        # Throw an error if this field has a non-null value and is a read-only field.
        if ($this->read_only and !is_null($this->value)) {
            throw new ValidationError(
                message: "Cannot assign value to read-only field `$this->name`.",
                return: "READ_ONLY_FIELD"
            );
        }
    }

    /**
     * Obtains an array of individual values for this field. If `many` is enabled, this method will ensure `value` is
     * already an array and simply return the array value. If `many` is not enabled, the individual `value` will be
     * wrapped in an array and returned. This intended to obtain ALL values that need to validated into
     * an array format for looping purposes.
     * @returns array An array containing all individual values that need validation.
     * @throws ValidationError When `many` is enabled, but `value` is not an array.
     */
    private function get_value_as_array() : array {
        # Check if many is enabled
        if ($this->many) {
            # Throw an error if the value is not already an array
            if (!is_array($this->value)) {
                throw new ValidationError(
                    message: "Field `$this->name` must be an array.",
                    return: "INVALID_MANY_VALUE"
                );
            }

            # Return the value array here if we didn't throw an error in the previous condition
            return $this->value;
        }

        # When many is not enabled, wrap the current `value` in an array so it can be used for looping.
        return [$this->value];
    }

    /**
     * Checks if this Field object has a valid `name` and `internal_name`. In the event that a `name` exists, but an
     * `internal_name` was not specified, the `internal_name` will be assigned the same value as `name` by this method.
     * @throws ServerError When this object's `name` has not been set.
     */
    private function check_field_names() {
        # Throw an error if no `name` has been set.
        if (empty($this->name)) {
            throw new ServerError(
                message: "Field object must be assigned a `name` value after it is constructed.",
                return: "UNNAMED_FIELD"
            );
        }

        # Default the `internal_name` to the field's `name` if it was not explicitly set.
        if (empty($this->internal_name)) {
            $this->internal_name = $this->name;
        }
    }

    /**
     * Checks if a given value's type is one of the available `types`.
     * @param mixed $value The value to check against available `types`.
     * $value is not an available choice.
     * @throws ValidationError If the specified value is not a supported type.
     */
    private function check_value_type(mixed $value) {
        # Raise an exception if the value is not a supported type.
        if (!in_array(gettype($value), $this->types, true)) {
            # Convert the type array to a string format and throw a type not supported
            $types_str = implode(", ", $this->types);
            throw new ValidationError(
                message: "Field `$this->name` must be one of types [$types_str]",
                return: "INVALID_TYPE"
            );
        }
    }

    /**
     * Checks if a given `value` is a valid `choice` option when `choices` are specified.
     * @param mixed $value The value to check against available choices.
     * @returns ValidationError When the $value is not an available choice.
     */
    private function check_value_choice(mixed $value) {
        # If choices are set, ensure this value is one of the choices
        if ($this->choices and !in_array($value, $this->choices)) {
            # Convert the choices array to a string format and throw a choice not supported
            $choices_str = implode(", ", $this->choices);
            throw new ValidationError(
                message: "Field `$this->name` must be one of [$choices_str]",
                return: "INVALID_CHOICE"
            );
        }
    }

    /**
     * Checks if a given `value` is an empty array or string and checks if empty values are allowed.
     * @param mixed $value The value to check for emptiness.
     * @returns ValidationError When the $value is not a supported type.
     */
    private function check_value_empty(mixed $value,) {
        # Check if value is an empty string or array. These are the only two types we consider possible to be empty
        # to avoid having to enable `allow_empty` for booleans, 0 integers, and null values
        # (which are checked separately).
        if (!$this->allow_empty and ($value === "" or $value === [])) {
            throw new ValidationError(
                message: "Field `$this->name` cannot be empty.",
                return: "EMPTY_NOT_ALLOWED"
            );
        }
    }

    /**
     * Checks if a given `value` is null and checks if null values are allowed.
     * @param mixed $value The value to check for emptiness.
     * @returns ValidationError When the $value is not a supported type.
     */
    private function check_value_null(mixed $value) {
        # Throw an error if the value is null and null values are not allowed.
        if (!$this->allow_null and is_null($value)) {
            throw new ValidationError(
                message: "Field `$this->name` cannot be nullified.",
                return: "NULL_NOT_ALLOWED"
            );
        }
    }
}