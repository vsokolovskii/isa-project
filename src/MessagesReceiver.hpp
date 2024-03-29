/* =========================================================================================================
   Case:      Brno University of Technology, ISA - Network Applications and Network Administration
   Date:      07.11.2021
   Author:    Vladislav Sokolovskii
   Contact:   xsokol15@stud.fit.vutbr.cz
   Description: This is the core class containing the methods which are responsible for the actual communication
                with the server
   ========================================================================================================== */

#pragma once

#include <csignal>
#include <map>
#include <vector>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

#include "ArgumentsParser.hpp"

#define MAX_PACKET_SIZE 2048

#define UNSUCCESS -1

#define FINAL_PERIOD 3

#define NOT_UPDATED 0

#define OLDMAILS ".oldmails"

//std::string is expected
#define SEND_REQUEST(request) { \
    if (_is_tls_established) { \
        if (SSL_write(ssl, request.c_str(), request.size()) <= 0) { \
            return false; \
        }          \
    } else { \
        if (BIO_write(bio, request.c_str(), request.size()) <= 0) { \
            return false; \
        } \
    } \
}


class MessagesReceiver {
public:
    ~MessagesReceiver();

    /**
     * This function establishes TCP communication according to the given parameters (-T|-S)
     * @return Returns true if everything went as expected, otherwise returns false
     */
    bool set_tcp_connection(ArgumentsParser&);

private:
    BIO *bio;
    SSL *ssl = nullptr;
    SSL_CTX* _ctx;
    ArgumentsParser* args_parser;

    bool _is_connected = false;
    bool _is_tls_established = false;
    bool _mails_are_being_read = false;


    /**
     *
     * @param bio - Basic I/O entity
     * @param period_indicator This flag should be set to true if multiple line response is expected from the server
     * otherwise it should be set to false
     * @return Returns the server response in the string format
     */
    std::string get_response(bool period_indicator);

    /**
     * This function initialize the SSL context
     */
    bool init_context();

    /**
     * This function parses the file containing authentication credentials
     * @param args_parser - TODO
     * @return Returns the tuple containing username and password for server authentication
     */
    std::tuple<std::string, std::string> parse_auth_file();

    /**
     * This function checks the prefix of the server response.
     * @param response from the server
     * @return Returns true if the response was +OK, otherwise returns false
     */
    static bool check_response_state(const std::string& response);

    /**
     * This function sends USER ... and PASS requests to the server and analyze the response.
     * @param bio - Basic I/O entity
     * @param username to authorize on the e-mail server, expected format is username@domain.com
     * @param password to authorize on the e-mail server
     * @return Returns true if the authorization was successful, otherwise returns false
     */
    bool authorize(BIO* bio, std::string username, std::string password);

    /**
     * @param bio - Basic I/O entity
     * @return Returns the number of e-mails in the inbox
     */
    int get_number_of_emails(BIO* bio);

    /**
     * This function processes e-mails one by one and, stores their message ids to the .oldmails file,
     * checks whether the e-mail is considered as new and deletes the e-mails if user has set the corresponding flag
     * @param bio - Basic I/O entity
     * @param total number of messages to process
     * @param out_dir directory to output the downloaded e-mails
     * @param args_parser ArgumentsParser entity
     * @return Returns the number of successfully downloaded and saved e-mails
     */
    int save_emails(BIO* bio, int total, const std::string& out_dir);

    /**
     * This function marks the e-mail with the given message number as deleted,
     * message will be deleted after the UPDATE state
     * @param bio - Basic I/O entity
     * @param msg_number to be deleted
     * @return Returns true if successful, otherwise returns false
     */
    bool delete_email(BIO* bio, int msg_number);

    /**
     * This function retrieves the unique message id of the given e-mail
     * @param e_mail
     * @return Returns the message id
     */
    std::string  get_message_id(std::string e_mail);

    /**
     * This function checks whether the given e-mail was already read by the popcl.
     * @param e_mail content to check
     * @return Returns true if the message is old (id of the given e-mail is in .oldmails file),
     * otherwise returns false
     */
    bool is_email_old(std::string e_mail);

    /**
     * This function checks whether the given e-mail id has already been stored
     * to the temporary .oldmails file, which consists of the already read e-mails.
     * @param e_mail content to check
     * @return Returns the empty string in case the e-mail was already read by the popcl before,
     * otherwise returns the message id of the e-mail
     */
    bool is_email_old(int i, std::string& msg_id);

    /**
     * This function sets the
     * @param ctx
     * @param args_parser
     * @return
     */
    int set_certificate_location();


    /**
     * This function was borrowed from https://www.fluentcpp.com/
     * Title: How to split a string in C++
     * Credit: Jonathan Boccara
     * Date: April 21, 2017
     * Availability: https://www.fluentcpp.com/2017/04/21/how-to-split-a-string-in-c/
     */
    std::vector<std::string> split(const std::string& s, char delimiter);
    /**
     * This function was borrowed from https://www.techiedelight.com/
     * Title: Trim a string in C++ – Remove leading and trailing spaces
     * Availability: https://www.techiedelight.com/trim-string-cpp-remove-leading-trailing-spaces/
     */
    std::string trim(const std::string &s);

    /**
     * This function loads message-ids of the downloaded e-mails to the internal map structure
     * for the fast access from during the program execution. The function is executed at the
     * beginning of the program.
     */
    void load_old_mails_map();
};

/**
 * This function handles the SIGTERM and SIGINT signals
 * @param signal number
 */
void signal_handler(int signal);

/**
 * This function stores message-ids of the downloaded e-mails to the .oldmails file
 * @return
 */
bool store_old_mails_file();
