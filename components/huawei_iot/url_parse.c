/*
 * @Author: your name
 * @Date: 2022-04-04 12:32:02
 * @LastEditTime: 2022-04-04 12:33:00
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \esp-idf-v4.3\examples\aithinker-esp32-huawei-iot\components\huawei_iot\url_parse.c
 */


#include "url_parse.h"

//  URL协议头表:用来判断解析出来的协议是否在此表中
char *URL_SCHEMES[] = {
    // official IANA registered schemes
    "aaa", "aaas", "about", "acap", "acct", "adiumxtra", "afp", "afs", "aim", "apt", "attachment", "aw",
    "beshare", "bitcoin", "bolo", "callto", "cap", "chrome", "crome-extension", "com-evenbrite-attendee",
    "cid", "coap", "coaps", "content", "crid", "cvs", "data", "dav", "dict", "lna-playsingle", "dln-playcontainer",
    "dns", "dtn", "dvb", "ed2k", "facetime", "fax", "feed", "file", "finger", "fish", "ftp", "geo", "gg", "git",
    "gizmoproject", "go", "gopher", "gtalk", "h323", "hcp", "http", "https", "iax", "icap", "icon", "im",
    "imap", "info", "ipn", "ipp", "irc", "irc6", "ircs", "iris", "iris.beep", "iris.xpc", "iris.xpcs", "iris.lws",
    "itms", "jabber", "jar", "jms", "keyparc", "lastfm", "ldap", "ldaps", "magnet", "mailserver", "mailto",
    "maps", "market", "message", "mid", "mms", "modem", "ms-help", "mssettings-power", "msnim", "msrp",
    "msrps", "mtqp", "mumble", "mupdate", "mvn", "news", "nfs", "ni", "nih", "nntp", "notes", "oid",
    "paquelocktoken", "pack", "palm", "paparazzi", "pkcs11", "platform", "pop", "pres", "prospero", "proxy",
    "psyc", "query", "reload", "res", "resource", "rmi", "rsync", "rtmp", "rtsp", "secondlife", "service", "session",
    "sftp", "sgn", "shttp", "sieve", "sip", "sips", "skype", "smb", "sms", "snews", "snmp", "soap.beep", "soap.beeps",
    "soldat", "spotify", "ssh", "steam", "svn", "tag", "teamspeak", "tel", "telnet", "tftp", "things", "thismessage",
    "tn3270", "tip", "tv", "udp", "unreal", "urn", "ut2004", "vemmi", "ventrilo", "videotex", "view-source", "wais", "webcal",
    "ws", "wss", "wtai", "wyciwyg", "xcon", "xcon-userid", "xfire", "xmlrpc.beep", "xmlrpc.beeps", "xmpp", "xri", "ymsgr",

    // unofficial schemes
    "javascript", "jdbc", "doi"};

char *
m_strdup(const char *str)
{
    int n = strlen(str) + 1;
    char *dup = malloc(n);
    if (dup)
        strcpy(dup, str);
    return dup;
}

static char *
get_part(char *url, const char *format, int l)
{
    bool has = false;

    char *tmp = malloc(URL_MAX_LENGTH * sizeof(char));
    memset(tmp, 0, URL_MAX_LENGTH * sizeof(char));
    char *fmt_url = m_strdup(url);
    char *ret = NULL;
    if (!tmp || !fmt_url)
        return NULL;

    strcpy(tmp, "");
    strcpy(fmt_url, "");

    // move pointer exactly the amount
    // of characters in the `prototcol` char
    // plus 3 characters that represent the `://`
    // part of the url
    fmt_url = fmt_url + l;
    sscanf(fmt_url, format, tmp);

    // if (0 != strcmp(tmp, tmp_url)) {
    if (0 != strcmp(tmp, fmt_url))
    {
        has = true;
        ret = m_strdup(tmp);
    }

    // descrement pointer to original
    // position so it can be free'd
    fmt_url = fmt_url - l;
    free(tmp);
    free(fmt_url);
    if (!has)
    {
        free(ret);
        return NULL;
    }
    else
    {
        return ret;
    }
}

/************* 解析URL各部分到url_data_t结构体 *************/
url_data_t *
url_parse(char *url)
{
    url_data_t *data = malloc(sizeof(url_data_t));
    if (!data)
        return NULL;

    data->href = url;
    char *tmp;
    char *tmp_url = m_strdup(url);
    bool is_ssh = false;

    /************ 解析协议头 ***********/
    char *protocol = url_get_protocol(tmp_url);
    if (!protocol)
        return NULL;
    // length of protocol plus ://
    int protocol_len = (int)strlen(protocol) + 3;
    data->protocol = protocol;

    is_ssh = url_is_ssh(protocol);

    /************ 解析用户名 ***********/
    char *auth = NULL;
    int auth_len = 0;
    if ((tmp = strstr(tmp_url, "@")))
    {
        auth = get_part(tmp_url, "%[^@]", protocol_len);
        auth_len = strlen(auth);
        if (auth)
            auth_len++;
    }

    data->auth = auth;
    char *hostname = NULL;

    /************ 解析主机名(包括端口号) ***********/
    hostname = (is_ssh)
                   ? get_part(tmp_url, "%[^:]", protocol_len + auth_len)
                   : get_part(tmp_url, "%[^/]", protocol_len + auth_len);

    if (!hostname)
        return NULL;
    int hostname_len = (int)strlen(hostname);
    char *tmp_hostname = m_strdup(hostname);
    data->hostname = hostname;

    /************ 解析主机名 ***********/
    char *host = malloc(strlen(tmp_hostname) * sizeof(char));
    memset(host, 0, strlen(tmp_hostname) * sizeof(char));
    sscanf(tmp_hostname, "%[^:]", host);
    if (!host)
        return NULL;
    int host_len = (int)strlen(host);
    data->host = host;

    /************ 解析端口号 ***********/
    char *port = malloc(URL_PROTOCOL_MAX_LENGTH * (sizeof(char)));
    memset(port, 0, URL_PROTOCOL_MAX_LENGTH * (sizeof(char)));
    if (!port)
        return NULL;

    tmp_hostname = tmp_hostname + (host_len + 1);
    sscanf(tmp_hostname, "%s", port);
    tmp_hostname = tmp_hostname - (host_len + 1);
    data->port = port;
    free(tmp_hostname);

    /************ 解析完整路径名 ***********/
    char *tmp_path;
    tmp_path = (is_ssh)
                   ? get_part(tmp_url, ":%s", protocol_len + auth_len + hostname_len)
                   : get_part(tmp_url, "/%s", protocol_len + auth_len + hostname_len);

    char *path = malloc(strlen(tmp_path) * sizeof(char) + 1);
    memset(path, 0, strlen(tmp_path) * sizeof(char) + 1);
    if (!path)
        return NULL;
    char *fmt = (is_ssh) ? "%s" : "/%s";
    sprintf(path, fmt, tmp_path);
    data->path = path;
    free(tmp_path);

    /************ 解析路径名(不包括参数) ***********/
    char *pathname = malloc(strlen(path) * sizeof(char) + 1);
    memset(pathname, 0, strlen(path) * sizeof(char) + 1);

    if (!pathname)
        return NULL;
    tmp_path = m_strdup(path);
    sscanf(tmp_path, "%[^? | ^#]", pathname);
    int pathname_len = strlen(pathname);
    data->pathname = pathname;

    /************* 解析搜索参数 ***********/
    char *search = malloc(URL_AUTH_MAX_LENGTH * sizeof(search));
    memset(search, 0, URL_AUTH_MAX_LENGTH * sizeof(search));
    if (!search)
        return NULL;
    tmp_path = tmp_path + pathname_len;
    sscanf(tmp_path, "%[^#]", search);
    tmp_path = tmp_path - pathname_len;
    data->search = search;
    int search_len = strlen(search);
    free(tmp_path);

    /************* 解析查询参数 ***********/
    char *query = malloc(URL_AUTH_MAX_LENGTH * sizeof(char));
    memset(query, 0, URL_AUTH_MAX_LENGTH * sizeof(char));
    if (!query)
        return NULL;
    sscanf(data->search, "?%s", query);
    data->query = query;

    /************* 解析hash值 ***********/
    char *hash = malloc(URL_AUTH_MAX_LENGTH * sizeof(char));
    memset(hash, 0, URL_AUTH_MAX_LENGTH * sizeof(char));
    if (!hash)
        return NULL;
    tmp_path = data->path + (pathname_len + search_len);
    sscanf(tmp_path, "%s", hash);
    tmp_path = data->path - (pathname_len + search_len);
    data->hash = hash;
    free(tmp_path);

    return data;
}

bool url_is_protocol(char *str)
{
    int count = sizeof(URL_SCHEMES) / sizeof(URL_SCHEMES[0]);

    for (int i = 0; i < count; ++i)
    {
        if (0 == strcmp(URL_SCHEMES[i], str))
        {
            return true;
        }
    }

    return false;
}

bool url_is_ssh(char *str)
{
    str = m_strdup(str);
    if (0 == strcmp(str, "ssh") ||
        0 == strcmp(str, "git"))
    {
        free(str);
        return true;
    }

    return false;
}

char *
url_get_protocol(char *url)
{
    char *protocol = malloc(URL_PROTOCOL_MAX_LENGTH * sizeof(char));
    if (!protocol)
        return NULL;
    sscanf(url, "%[^://]", protocol);
    if (url_is_protocol(protocol))
        return protocol;
    return NULL;
}

char *
url_get_auth(char *url)
{
    char *protocol = url_get_protocol(url);
    if (!protocol)
        return NULL;
    int l = (int)strlen(protocol) + 3;
    return get_part(url, "%[^@]", l);
}

char *
url_get_hostname(char *url)
{
    int l = 3;
    char *protocol = url_get_protocol(url);
    char *tmp_protocol = m_strdup(protocol);
    char *auth = url_get_auth(url);

    if (!protocol)
        return NULL;
    if (auth)
        l += strlen(auth) + 1; // add one @ symbol
    if (auth)
        free(auth);

    l += (int)strlen(protocol);

    free(protocol);

    char *hostname = url_is_ssh(tmp_protocol)
                         ? get_part(url, "%[^:]", l)
                         : get_part(url, "%[^/]", l);
    free(tmp_protocol);
    return hostname;
}

char *
url_get_host(char *url)
{
    char *host = malloc(URL_HOSTNAME_MAX_LENGTH * sizeof(char));
    char *hostname = url_get_hostname(url);
    memset(host, 0, URL_HOSTNAME_MAX_LENGTH * sizeof(char));
    if (!host || !hostname)
        return NULL;

    sscanf(hostname, "%[^:]", host);

    free(hostname);

    return host;
}

char *
url_get_pathname(char *url)
{
    char *path = url_get_path(url);
    char *pathname = malloc(URL_MAX_LENGTH * sizeof(char));
    memset(pathname, 0, URL_MAX_LENGTH * sizeof(char));
    if (!path || !pathname)
        return NULL;

    sscanf(path, "%[^?]", pathname);

    free(path);

    return pathname;
}

char *
url_get_path(char *url)
{
    int l = 3;
    char *tmp_path;
    char *protocol = url_get_protocol(url);
    char *auth = url_get_auth(url);
    char *hostname = url_get_hostname(url);

    if (!protocol || !hostname)
        return NULL;

    bool is_ssh = url_is_ssh(protocol);

    l += (int)strlen(protocol) + (int)strlen(hostname);

    if (auth)
        l += (int)strlen(auth) + 1; // @ symbol

    tmp_path = (is_ssh)
                   ? get_part(url, ":%s", l)
                   : get_part(url, "/%s", l);

    char *fmt = (is_ssh) ? "%s" : "/%s";
    char *path = malloc(strlen(tmp_path) * sizeof(char) + 1);
    memset(path, 0, strlen(tmp_path) * sizeof(char) + 1);
    sprintf(path, fmt, tmp_path);

    if (auth)
        free(auth);
    free(protocol);
    free(hostname);
    free(tmp_path);

    return path;
}

char *
url_get_search(char *url)
{
    char *path = url_get_path(url);
    char *pathname = url_get_pathname(url);
    char *search = malloc(URL_AUTH_MAX_LENGTH * sizeof(char));
    memset(search, 0, URL_AUTH_MAX_LENGTH * sizeof(char));
    if (!path || !search)
        return NULL;

    path = path + (int)strlen(pathname);
    sscanf(path, "%[^#]", search);
    path = path - (int)strlen(pathname);

    free(path);
    free(pathname);

    return search;
}

char *
url_get_query(char *url)
{
    char *search = url_get_search(url);
    char *query = malloc(URL_AUTH_MAX_LENGTH * sizeof(char));
    memset(query, 0, URL_AUTH_MAX_LENGTH * sizeof(char));
    if (!search)
        return NULL;
    sscanf(search, "?%s", query);
    free(search);
    return query;
}

char *
url_get_hash(char *url)
{
    char *hash = malloc(URL_AUTH_MAX_LENGTH * sizeof(char));
    memset(hash, 0, URL_AUTH_MAX_LENGTH * sizeof(char));
    if (!hash)
        return NULL;

    char *path = url_get_path(url);
    if (!path)
        return NULL;

    char *pathname = url_get_pathname(url);
    if (!pathname)
        return NULL;
    char *search = url_get_search(url);

    int pathname_len = (int)strlen(pathname);
    int search_len = (int)strlen(search);

    path = path + (pathname_len + search_len);
    sscanf(path, "%s", hash);
    path = path - (pathname_len + search_len);
    free(pathname);
    free(path);
    if (search)
        free(search);

    return hash;
}

char *url_get_port(char *url)
{
    char *port = malloc(URL_PROTOCOL_MAX_LENGTH * sizeof(char));
    memset(port, 0, URL_PROTOCOL_MAX_LENGTH * sizeof(char));

    char *hostname = url_get_hostname(url);
    char *host = url_get_host(url);

    if (!port || !hostname)
        return NULL;

    char *tmp_hostname = hostname;
    tmp_hostname = hostname + (strlen(host) + 1);
    sscanf(tmp_hostname, "%s", port);

    free(hostname);
    return port;
}

void url_inspect(char *url)
{
    url_data_inspect(url_parse(url));
}

void url_data_inspect(url_data_t *data)
{
    printf("#url =>\n");
    printf("    .href: \"%s\"\n", data->href);
    printf("    .protocol: \"%s\"\n", data->protocol);
    printf("    .host: \"%s\"\n", data->host);
    printf("    .auth: \"%s\"\n", data->auth);
    printf("    .hostname: \"%s\"\n", data->hostname);
    printf("    .pathname: \"%s\"\n", data->pathname);
    printf("    .search: \"%s\"\n", data->search);
    printf("    .path: \"%s\"\n", data->path);
    printf("    .hash: \"%s\"\n", data->hash);
    printf("    .query: \"%s\"\n", data->query);
    printf("    .port: \"%s\"\n", data->port);
}

void url_free(url_data_t *data)
{
    if (!data)
        return;
    if (data->auth)
        free(data->auth);
    if (data->protocol)
        free(data->protocol);
    if (data->hostname)
        free(data->hostname);
    if (data->host)
        free(data->host);
    if (data->pathname)
        free(data->pathname);
    if (data->path)
        free(data->path);
    if (data->hash)
        free(data->hash);
    if (data->search)
        free(data->search);
    if (data->query)
        free(data->query);
}