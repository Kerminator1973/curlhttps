/*jshint esversion: 6 */

// https://www.npmjs.com/package/express-xml-bodyparser

var express = require('express'),
	app = express(),
	xmlparser = require('express-xml-bodyparser');

// Планируем извлекать XML из тела документа.
// Критично, чтобы у данных был корректный заголовок, начинающийся с:
//		<?xml version=\"1.0\" encoding=\"UTF-8\"?>
app.use(xmlparser());

app.get('*', function(req, res, next) {
	res.send(`
		<xml>
			<ok>Stratosfera</ok>
		</xml>
	`);
});

app.post('*', function(req, res, next) {

	if(req.body) {

		// Все запросы от АДМ содержат тэг message
		if(req.body.message) {

			// Описание протокола:
			// "\PDM\ПроАТМ\1. Trunk\1.3 Эксплуатационная документация\ЭД. АДМ - Технические спецификации. ПроАТМ"
			var _uid = '';
			var _xtype = '';
			var _member_id = '';

			if(req.body.message.$) {

				_uid = req.body.message.$.uid;
				_xtype = req.body.message.$.xtype;

				console.log('uid =' + _uid + ", xtype = " + _xtype);
			}

			if(req.body.message.sender) {

				var _sender = req.body.message.sender;

				try
				{
					if(_sender[0].$) {

						_member_id = _sender[0].$['member-id'];
						console.log('member-id = ' + _member_id);
					}
				}
				catch(e)
				{
					console.log('Exception was happened: ' + e);
					_member_id = "10000001";
				}
			}

			if(req.body.message.request) {

				var _request = req.body.message.request;
				console.log(JSON.stringify(_request));

				//var client_account = _request.$['client-account'];
				//if(client_account) {
				//	console.log('Client\'s account: ' + client_account);
				//}
			}

			// Готовим ответы на запросы сервера
			if(_uid && _member_id) {

				console.log('Happy simulation: OK');

				if(_xtype === "ADM001" || _xtype === "ADM004") {


					if( _uid === "00000000" ||_xtype === "ADM004") {

						// Возвращаем "счастливый" ответ на технический запрос
						res.send(`
						<? xml version="1.0" encoding="UTF-8" ?> 
							<message date-time="2011-11-01 13:01:00" uid="${_uid}" xtype="${_xtype}">
							<sender member-id="${_member_id}"/>
							<response>
								<status>1</status>
								<err></err>
							</response>
							</message>
						`);

						return;
					}

					// Для честного УИН-а возвращаем документ с названием организации и списком счетов
					res.send(`
					<? xml version="1.0" encoding="UTF-8" ?> 
						<message date-time="2011-11-01 13:01:00" uid="${_uid}" xtype="${_xtype}">
						<sender member-id="${_member_id}"/>
						<response>
							<status>1</status>
							<err></err>
							<company_name>The Atlantis company</company_name>
							<inn>101990004587201</inn>
							<cash_depositor_surname>Кукуев А.В.</cash_depositor_surname>
							<accounts>
								<account>KZ73914398415PC04015</account>
								<account>KZ73914398415PC04016</account>
								<account>KZ73914398415PC04049</account>
								<account>KZ73914398415PC05001</account>
								<account>KZ73914398415PC04001</account>
								<account>KZ73914398415PC04002</account>
								<account>KZ73914398415PC04003</account>
								<account>KZ73914398415PC04004</account>
								<account>KZ73914398415PC04006</account>
								<account>KZ73914398415PC04007</account>
							</accounts>
							<payment_purposes>
								<purpose>
									<title>ПОГАШЕНИЕ ССУДЫ</title>
									<code>R102</code>
								</purpose>
								<purpose>
									<title>НАЛОГ</title>
									<code>TAXES</code>
								</purpose>
								<purpose>
									<title>Поставщики</title>
									<code>VENDORS</code>
								</purpose>
								<purpose>
									<title>Штрафы</title>
									<code>FINES</code>
								</purpose>
								<purpose>
									<title>Пенсионный фонд</title>
									<code></code>
								</purpose>
								<purpose>
									<title>Медицинская страховка</title>
									<code>MEDI-CARE</code>
								</purpose>
								<purpose>
									<title>Прокатилло</title>
									<code>EASYMONEY</code>
								</purpose>
							</payment_purposes>
						</response>
						</message>
					`);

					return;
				}
				else if(_xtype === "ADM002") {

					// Возвращаем "счастливый" ответ на финансовый запрос
					res.send(`
					<? xml version="1.0" encoding="UTF-8" ?> 
						<message date-time="2011-11-01 13:01:00" uid="${_uid}" xtype="ADM002">
						<sender member-id="${_member_id}"/>
					  	<response>
					  		<status>1</status>
					  		<trx-datetime>2011-11-01 12:58:08</trx-datetime>
					  		<err></err>
					 	</response>
					</message>
					`);

					return;
				}
			}
		}
	}

	res.send(`
		<xml>
			<ok>Stratosfera</ok>
		</xml>
	`);
});

// Запускаем сервер, работающий по https. Сертификаты должны
// размещаться в корневом подкаталоге. Перед запуском сервера в режиме
// https, следует скопировать файлы "server_dev.key" и "server_dev.crt"
// из подкаталога "\MakeCertificates\" в текущий подкаталог
//
// Статьи:
//  https://medium.com/@noumaan/ssl-app-dev-a2923d5113c6
//  https://www.sitepoint.com/how-to-use-ssltls-with-node-js/
//  https://hackernoon.com/set-up-ssl-in-nodejs-and-express-using-openssl-f2529eab5bb

const https = require("https"),
fs = require("fs");

const options = {
  key: fs.readFileSync("localhost.key"),
  cert: fs.readFileSync("localhost.crt")
};

let server = https.createServer(options, app).listen(3000, function() {

  let host = server.address().address;
  let port = server.address().port;
  console.log('The server listening at ' + host + ':' + port);  
});
