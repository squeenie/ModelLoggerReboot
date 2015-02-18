//#include <iostream>
//#include <string.h>
//
//using namespace std;
//
//struct data
//{
//	string email;
//	int id;
//};
//
//int gethash(string & email);
//
//int main()
//{
//	data database;
//	cin >> database.email;
//	database.id = 1;
//	cout << database.email << endl;
//	cout << gethash(database.email) << endl;
//	return 0;
//}
//
//int gethash(string & email)
//{
//	int hash = 0;
//	int i = 0;
//
//	for (i = 0; i < email.length(); ++i)
//	{
//		hash = (hash * email[i] + 3) % 100;
//	}
//	return hash;
//}
