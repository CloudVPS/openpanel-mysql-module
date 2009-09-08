%define version 0.9.4

%define libpath /usr/lib
%ifarch x86_64
  %define libpath /usr/lib64
%endif

Summary: MySQL database module
Name: openpanel-mod-mysql
Version: %version
Release: 1
License: GPLv2
Group: Development
Source: http://packages.openpanel.com/archive/openpanel-mod-mysql-%{version}.tar.gz
Patch1: openpanel-mod-mysql-00-makefile
BuildRoot: /var/tmp/%{name}-buildroot
Requires: openpanel-core >= 0.8.3
Requires: openpanel-mod-user
Requires: mysql-server
Requires: openssl

%description
MySQL database module
Openpanel mysql management module

%prep
%setup -q -n openpanel-mod-mysql-%version
%patch1 -p0 -b .buildroot

%build
BUILD_ROOT=$RPM_BUILD_ROOT
./configure
make

%install
BUILD_ROOT=$RPM_BUILD_ROOT
rm -rf ${BUILD_ROOT}
mkdir -p ${BUILD_ROOT}/var/opencore/modules/MySQL.module
mkdir -p ${BUILD_ROOT}/etc/openpanel
cp -rf ./mysqlmodule.app ${BUILD_ROOT}/var/opencore/modules/MySQL.module/
cp *.png ${BUILD_ROOT}/var/opencore/modules/MySQL.module/
ln -sf mysqlmodule.app/exec ${BUILD_ROOT}/var/opencore/modules/MySQL.module/action
cp module.xml ${BUILD_ROOT}/var/opencore/modules/MySQL.module/module.xml
install -m 755 verify ${BUILD_ROOT}/var/opencore/modules/MySQL.module/verify

%post
mkdir -p /var/opencore/conf/staging/MySQL
chown opencore:authd /var/opencore/conf/staging/MySQL
if [ ! -e /etc/openpanel/mysql.pwd ]; then

OP_UPW=`openssl rand -base64 12`
MYSQL_QRY="GRANT ALL PRIVILEGES ON *.* TO 'openpanel'@'localhost' IDENTIFIED BY '"$OP_UPW"' WITH GRANT OPTION;"

chkconfig --level 2345 mysqld on
service mysqld restart >/dev/null 2>&1
mysql -u root -e "$MYSQL_QRY"

echo $OP_UPW > /etc/openpanel/mysql.pwd
chmod 600 /etc/openpanel/mysql.pwd
fi

%files
%defattr(-,root,root)
/
