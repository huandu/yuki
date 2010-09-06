DROP TABLE IF EXISTS mysample;
CREATE TABLE mysample (
    id bigint auto_increment primary key,
    uid varchar(32) not null,
    int_value int not null,
    bigint_unsigned_value bigint unsigned not null,
    char_value char(16) not null,
    varchar_value varchar(32) not null,
    text_value text,
    unique key I_uid (uid)
) engine=InnoDB character set=utf8;
