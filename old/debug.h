
/* Appends a pretty printed hex data dump of the data in slice. This can be
 * optionally coloured using inline ansi terminal colour commands.
 */
DMEM_API void dv_hex_dump(d_vector(char)* s, d_string data, bool colors);

/* Logs the data as a hex to stderr with format and the variable arguments
 * specifying the header.
 */
DMEM_API void dv_log(d_string data, const char* format, ...);

/* ------------------------------------------------------------------------- */

