#ifndef _JSON_H
#define _JSON_H

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/pointer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace json
{

class Json
{
private:
    rapidjson::Document m_document;

    /**
     * @brief Construct a new Json object form a rapidjason::Value.
     * Copies the value.
     *
     * @param value The rapidjson::Value to copy.
     */
    Json(const rapidjson::Value& value);

    /**
     * @brief Construct a new Json object form a rapidjason::GenericObject.
     * Copies the object.
     *
     * @param object The rapidjson::GenericObject to copy.
     */
    Json(const rapidjson::GenericObject<true, rapidjson::Value>& object);

public:
    /**
     * @brief Construct a new Json empty json object.
     *
     */
    Json();

    /**
     * @brief Construct a new Json object from a rapidjason Document.
     * Moves the document.
     *
     * @param document The rapidjson::Document to move.
     */
    explicit Json(rapidjson::Document&& document);

    /**
     * @brief Construct a new Json object from a json string
     *
     * @param json The json string to parse.
     */
    explicit Json(const char* json);

    /**
     * @brief Copy constructs a new Json object.
     * Value is copied.
     *
     * @param other The Json to copy.
     */
    Json(const Json& other);

    /**
     * @brief Copy assignment operator.
     * Value is copied.
     *
     * @param other The Json to copy.
     * @return Json& The new Json object.
     */
    Json& operator=(const Json& other);

    bool operator==(const Json& other) const;

    /************************************************************************************/
    // Static Helpers
    /************************************************************************************/

    /**
     * @brief Transform dot path string to pointer path string.
     *
     * @param dotPath The dot path string.
     * @return std::string The pointer path string.
     */
    static std::string formatJsonPath(std::string_view dotPath);

    /************************************************************************************/
    // Runtime functionality, used only by our operations.
    // TODO: Move runtime functionality to separate class.
    /************************************************************************************/

    /**
     * @brief Move copy constructor.
     * Value is moved.
     *
     * @param other The Json to move, which is left in an empty state.
     */
    Json(Json&& other) noexcept;

    /**
     * @brief Move copy assignment operator.
     * Value is moved.
     *
     * @param other The Json to move, which is left in an empty state.
     * @return Json& The new Json object.
     */
    Json& operator=(Json&& other) noexcept;

    /**
     * @brief Check if the Json contains a field with the given pointer path.
     *
     * @param pointerPath The pointer path to check.
     * @return true The Json contains the field.
     * @return false The Json does not contain the field.
     *
     * @throws std::runtime_error If the pointer path is invalid.
     */
    bool exists(std::string_view pointerPath) const;

    /**
     * @brief Check if the Json contains a field with the given dot path, and if so, with
     * the given value.
     *
     * @param pointerPath
     * @param value
     * @return true The Json contains the field with the given value.
     * @return false The Json does not contain the field with the given value.
     *
     * @throws std::runtime_error If the pointer path is invalid.
     */
    bool equals(std::string_view pointerPath, const Json& value) const;

    /**
     * @brief Check if basePointerPath field's value is equal to referencePointerPath
     * field's value. If basePointerPath or referencePointerPath is not found, returns
     * false.
     *
     * @param basePointerPath The base pointer path to check.
     * @param referencePointerPath The reference pointer path to check.
     * @return true The base field's value is equal to the reference field's value.
     * @return false The base field's value is not equal to the reference field's value.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    bool equals(std::string_view basePointerPath,
                std::string_view referencePointerPath) const;

    /**
     * @brief Set the value of the field with the given pointer path.
     * Overwrites previous value.
     *
     * @param pointerPath The pointer path to set.
     * @param value The value to set.
     *
     * @throws std::runtime_error If the pointer path is invalid.
     */
    void set(std::string_view pointerPath, const Json& value);

    /**
     * @brief Set the value of the base field with the value of the reference field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     * @param referencePointerPath The reference pointer path.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    void set(std::string_view basePointerPath, std::string_view referencePointerPath);

    /************************************************************************************/
    // Getters
    /************************************************************************************/

    /**
     * @brief get the value of the string field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<std::string> getString(std::string_view path = "") const;
    /**
     * @brief get the value of the int field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<int> getInt(std::string_view path = "") const;

    /**
     * @brief get the value of the double field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<double> getDouble(std::string_view path = "") const;

    /**
     * @brief get the value of the bool field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<bool> getBool(std::string_view path = "") const;

    /**
     * @brief get the value of the array field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<std::vector<Json>> getArray(std::string_view path = "") const;

    /**
     * @brief get the value of the object field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<std::vector<std::tuple<std::string, Json>>>
    getObject(std::string_view path = "") const;
    /**
     * @brief Get Json prettyfied string.
     *
     * @return std::string The Json prettyfied string.
     */
    std::string prettyStr() const;

    /**
     * @brief Get Json string.
     *
     * @return std::string The Json string.
     */
    std::string str() const;

    /**
     * @brief Get Json string from an object.
     *
     * @param path The path to the object.
     * @return std::string The Json string or nothing if the path not found.
     * @throws std::runtime_error If the path is invalid.
     */
    std::optional<std::string> str(std::string_view path) const;

    friend std::ostream& operator<<(std::ostream& os, const Json& json);

    /************************************************************************************/
    // Query
    /************************************************************************************/

    /**
     * @brief Get number of elements.
     * If array get number of elements. If object get number of pairs (key, value).
     *
     * @return size_t The number of elements.
     *
     * @throws std::runtime_error If the Json is not an array or object.
     */
    size_t size(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is Null.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Null.
     * @return false if Json is not Null.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isNull(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is Bool.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Bool.
     * @return false if Json is not Bool.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isBool(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is Number.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Number.
     * @return false if Json is not Number.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isNumber(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is integer.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Int.
     * @return false if Json is not Int.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isInt(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is double.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Double.
     * @return false if Json is not Double.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isDouble(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is String.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is String.
     * @return false if Json is not String.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isString(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is Array.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Array.
     * @return false if Json is not Array.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isArray(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is Object.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Object.
     * @return false if Json is not Object.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isObject(std::string_view path = "") const;

    /**
     * @brief Get the type name of the Json.
     *
     * @return std::string The type name of the Json.
     */
    std::string typeName() const;

    /************************************************************************************/
    // Setters
    /************************************************************************************/

    /**
     * @brief Set the Null object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setNull(std::string_view path = "");

    /**
     * @brief Set the Boolean object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setBool(bool value, std::string_view path = "");

    /**
     * @brief Set the Integer object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setInt(int value, std::string_view path = "");

    /**
     * @brief Set the Double object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setDouble(double value, std::string_view path = "");

    /**
     * @brief Set the String object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setString(std::string_view value, std::string_view path = "");

    /**
     * @brief Set the Array object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setArray(std::string_view path = "");

    /**
     * @brief Set the Object object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setObject(std::string_view path = "");

    /**
     * @brief Append string to the Array object at the path.
     * Parents objects are created if they do not exist.
     * If the object is not an Array, it is converted to an Array.
     *
     * @param value The string to append.
     *
     * @throws std::runtime_error If path is invalid.
     */
    void appendString(std::string_view value, std::string_view path = "");

    /**
     * @brief Erase Json object at the path.
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if object was erased, false if object was not found.
     *
     * @throws std::runtime_error If path is invalid.
     */
    bool erase(std::string_view path = "");
};

} // namespace json

#endif // _JSON_H
