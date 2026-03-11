#include "ctp_trade_handler.h"
#include "INIReader.h"
#include <iostream>
#include <string>
#include <cstring>
#include <vector>

using namespace std;

static void copy_str(char* dest, size_t dest_size, const std::string& src)
{
    if (dest == nullptr || dest_size == 0) return;
    std::strncpy(dest, src.c_str(), dest_size - 1);
    dest[dest_size - 1] = '\0';
}

int main(int argc, char* argv[])
{
    INIReader reader("../conf/ctp.ini");
    if (reader.ParseError() != 0) {
        std::cout << "Can't load '../conf/ctp.ini'" << std::endl;
        return 1;
    }

    std::string broker_id   = reader.Get("user", "BrokerID", "");
    std::string user_id     = reader.Get("user", "UserID", "");
    std::string password    = reader.Get("user", "Password", "");
    std::string new_passwd  = reader.Get("user", "SetPasswd", "");
    std::string app_id      = reader.Get("user", "AppID", "");
    std::string auth_code   = reader.Get("user", "AuthCode", "");
    std::string front_addr  = reader.Get("td",   "FrontAddr", "");

    if (broker_id.empty()) {
        std::cout << "config error: [user] BrokerID is empty" << std::endl;
        return 1;
    }
    if (user_id.empty()) {
        std::cout << "config error: [user] UserID is empty" << std::endl;
        return 1;
    }
    if (password.empty()) {
        std::cout << "config error: [user] Password is empty" << std::endl;
        return 1;
    }
    if (new_passwd.empty()) {
        std::cout << "config error: [user] SetPasswd is empty" << std::endl;
        return 1;
    }
    if (front_addr.empty()) {
        std::cout << "config error: [td] FrontAddr is empty" << std::endl;
        return 1;
    }

    std::cout << "BrokerID  : " << broker_id << std::endl;
    std::cout << "UserID    : " << user_id << std::endl;
    std::cout << "FrontAddr : " << front_addr << std::endl;

    ctp_trade_handle ctp;
    ctp.CreateFtdcTraderApi();

    std::vector<char> front_addr_buf(front_addr.begin(), front_addr.end());
    front_addr_buf.push_back('\0');
    ctp.RegisterFront(front_addr_buf.data());

    ctp.init();

    if (!app_id.empty() && !auth_code.empty()) {
        CThostFtdcReqAuthenticateField reqAuthenticate = {0};
        copy_str(reqAuthenticate.BrokerID, sizeof(reqAuthenticate.BrokerID), broker_id);
        copy_str(reqAuthenticate.UserID, sizeof(reqAuthenticate.UserID), user_id);
        copy_str(reqAuthenticate.AppID, sizeof(reqAuthenticate.AppID), app_id);
        copy_str(reqAuthenticate.AuthCode, sizeof(reqAuthenticate.AuthCode), auth_code);

        std::cout << "send authenticate request..." << std::endl;
        if (ctp.ReqAuthenticate(&reqAuthenticate, 1) != 0) {
            std::cout << "authenticate failed" << std::endl;
            ctp.exit();
            return 1;
        }
    } else {
        std::cout << "AppID/AuthCode empty, skip authenticate." << std::endl;
    }

    CThostFtdcReqUserLoginField reqUserLogin = {0};
    copy_str(reqUserLogin.BrokerID, sizeof(reqUserLogin.BrokerID), broker_id);
    copy_str(reqUserLogin.UserID, sizeof(reqUserLogin.UserID), user_id);
    copy_str(reqUserLogin.Password, sizeof(reqUserLogin.Password), password);

    std::cout << "send login request..." << std::endl;
    int login_ret = ctp.ReqUserLogin(&reqUserLogin, 2);
    if (login_ret != 0) {
        std::cout << "login failed, continue password update in current session..." << std::endl;
    }

    CThostFtdcUserPasswordUpdateField reqUserPasswordUpdate = {0};
    copy_str(reqUserPasswordUpdate.BrokerID, sizeof(reqUserPasswordUpdate.BrokerID), broker_id);
    copy_str(reqUserPasswordUpdate.UserID, sizeof(reqUserPasswordUpdate.UserID), user_id);
    copy_str(reqUserPasswordUpdate.OldPassword, sizeof(reqUserPasswordUpdate.OldPassword), password);
    copy_str(reqUserPasswordUpdate.NewPassword, sizeof(reqUserPasswordUpdate.NewPassword), new_passwd);

    std::cout << "send password update request..." << std::endl;
    if (ctp.ReqUserPasswordUpdate(&reqUserPasswordUpdate, 3) != 0) {
        std::cout << "password update failed" << std::endl;
        ctp.exit();
        return 1;
    }

    std::cout << "password update success" << std::endl;
    std::cout << "now update ../conf/ctp.ini: Password = SetPasswd, then rerun to verify login." << std::endl;

    std::cin.get();
    ctp.exit();
    return 0;
}