//------------------------------------
// BugfixedAPI
// Requires BugfixedAPI AMXX module
// https://github.com/tmp64/BugfixedHL
//------------------------------------

enum E_ClientSupports
{
	AGHL_SUPPORTS_NONE = 0,
	AGHL_SUPPORTS_UNICODE_MOTD = (1 << 0),
	AGHL_SUPPORTS_HTML_MOTD = (1 << 1)
};

/**
 * Returns value of player's "aghl_supports" convar.
 * @param idx			Player's index
 * @return				0 or 1
 */
native E_ClientSupports:bhl_get_player_supports(idx);

/**
 * Returns whether or not player has color code parsing enabled.
 * @param idx			Player's index
 * @return				0 or 1
 */
native bhl_is_color_text_enabled(idx);

/**
 * Copies player's client version string to a buffer
 * @param idx			Player's index
 * @param buf			Buffer to store the string
 * @param size			Buffer size
 * @return				1 on success, 0 otherwise (player is not connected or aghl_version is invalid)
 */
native bhl_get_client_version_string(idx, buf[], size = sizeof(buf));

/**
 * Copies player's client version string to buf
 * @param idx			Player's index
 * @param major			Variable to store version major in
 * @param minor			Variable to store version minor in
 * @param patch			Variable to store version patch in
 * @return				1 on success, 0 otherwise (player is not connected or aghl_version is invalid)
 */
native bhl_get_client_version(idx, &major, &minor, &patch);

/**
 * Returns whether or not player's client build is dirty
 * @param idx			Player's index
 * @return				0 or 1
 */
native bhl_is_client_dirty(idx);

/**
 * Copies player's client version commit to a buffer
 * @param idx			Player's index
 * @param buf			Buffer to store the string
 * @param size			Buffer size
 * @return				1 on success, 0 otherwise (player is not connected or aghl_version is invalid)
 */
native bhl_get_client_version_commit(idx, buf[], size = sizeof(buf));
