create view Q6 as select name from members natural join passes where duration = 'Quarterly Pass';