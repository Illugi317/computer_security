
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char * secret_message = "OLZLJYLATLZZHNLPZHNYLLHISLULZZLZ";
const char * secret_passphrase = "my_secret_passphrase";

const char * const get_input(void);
const char * const get_decoded_message(const char * const);

int main()
{
    printf("Enter the secret passphrase that will decrypt the secret message: ");
    fflush(stdout);
    const char * const pass_phrase = get_input();
    printf("decoded message: %s\n", get_decoded_message(pass_phrase));
    return 0;
} 

const char * const get_input(void)
{
	char buf[256];
	gets(buf);
	return strdup(buf); 
}

const char * const get_decoded_message(const char * const pass_phrase)
{
    if(!strcmp(pass_phrase, secret_passphrase))
    {
        return "DECODED MESSAGE"; // only a placeholder, here decryption would take place
    }

    return "PASSPHRASE WAS WRONG";
}

void print_secret_message(void) {
    printf("%s\n", secret_message);
    exit(0);
}