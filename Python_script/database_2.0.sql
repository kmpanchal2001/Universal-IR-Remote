show databases;

create database remotevalue1;

use remotevalue1;

create table hexdata1(remote varchar(30), val varchar(30));

insert into hexdata1 values ('POWER', '0x1FE48B7'),
						   ('MODE','0x1FE58A7'),
                           ('MUTE','0x1FE7887'),
                           ('PLAY/PAUSE','0x1FE807F'),
                           ('PRIVIOUS','0x1FE40BF'),
                           ('NEXT','0x1FEC03F'),
                           ('EQ', '0x1FE20DF'),
                           ('VOLUME-', '0x1FEA05F'),
                           ('VOLUME+', '0x1FE609F'),
                           ('RPT', '0x1FE10EF'),
                           ('U/SD', '0x1FE906F'),
                           ('0', '0x1FEE01F'),
                           ('1', '0x1FE50AF'),
                           ('2', '0x1FED827'),
                           ('3', '0x1FEF807'),
                           ('4', '0x1FE30CF'),
                           ('5', '0x1FEB04F'),
                           ('6', '0x1FE708F'),
                           ('7', '0x1FE00FF'),
                           ('8', '0x1FEF00F'),
                           ('9', '0x1FE9867');

select * from hexdata1;