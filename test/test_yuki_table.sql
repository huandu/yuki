DROP TABLE IF EXISTS mytest;
CREATE TABLE mytest (
    id bigint auto_increment primary key,
    uid varchar(32) binary,
    cash bigint not null,
    diamond bigint not null,
    content varchar(255) not null,
    created_at timestamp not null default CURRENT_TIMESTAMP,
    unique key I_uid (uid)
) engine=InnoDB character set=utf8;
