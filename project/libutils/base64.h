#ifndef __SUPPORT_BASE64_H__
#define __SUPPORT_BASE64_H__

#ifdef __cplusplus
extern "C" {
#endif    

/** @file
 base64 helper functions
 */

/** 
 * Decodes a 64base encoded memory. The decoded memory is
 * expected to be freed by the caller by calling `free()`
 *
 * @returns the length of the out buffer
 *
 @since v0.8.1
 */
int base64Decode(const unsigned char *in, unsigned int inLength, unsigned char **out);
    
/**
 * Encodes bytes into a 64base encoded memory with terminating '\0' character. 
 * The encoded memory is expected to be freed by the caller by calling `free()`
 *
 * @returns the length of the out buffer
 *
 @since v2.1.4
 */
int base64Encode(const unsigned char *in, unsigned int inLength, char **out);

#ifdef __cplusplus
}
#endif    

#endif // __SUPPORT_BASE64_H__
