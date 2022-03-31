var buttons = [
       'computerbtn',
       'documentbtn',
       'picturebtn',
       'musicbtn',
       'videobtn',
       'downloadbtn',

	'settingsbtn',
	'powerbtn'
];

for(var idx = 0; idx < buttons.length; idx++) {
    执行命令("/usr/bin/dde-launcher", "-s")
    点击按钮('byAccName', buttons[idx]);
}