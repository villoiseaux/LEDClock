<h1>Concerning local files</h1>
In order to compile the project it is necessary to have a "local" directory.
This directory whill contain files that you don't want to store in GIT repositories by PUSH.
This is configured in .gitignore file.

<h2>The SSID.h file </h2>
If may content :
<pre>
#define PRIMARY_SSID	"LAX", "xxx"
#define SECONDARY_SSID	"PPT", "zzz"
</pre>
With SSID and password used by default

<h2>The Abstract.h file </h2>
If may content :
<pre>
#define ABSTRACT_URL "https://ipgeolocation.abstractapi.com/v1/?api_key=AAAAAAAAAA&fields=timezone"
</pre>
Where AAAAAAAAAA is your API KEY
