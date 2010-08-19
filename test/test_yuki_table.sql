DROP TABLE IF EXISTS mytest;
CREATE TABLE mytest (
    uid varchar(32) binary primary key,
    cash bigint not null,
    diamond bigint not null,
    created_at timestamp not null default CURRENT_TIMESTAMP
) engine=InnoDB character set=utf8;
