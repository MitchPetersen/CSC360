DROP TABLE Members
CREATE TABLE Members
(
  Name VARCHAR(25) NOT NULL,
  Mailing_Address VARCHAR(50) NOT NULL,
  Status VARCHAR(15) NOT NULL,
  Contact_Information VARCHAR(12) NOT NULL,
  Fitness_Center_ID INT NOT NULL,
  Membership_Type VARCHAR(15) NOT NULL,
  PRIMARY KEY (Fitness_Center_ID)
);

CREATE TABLE Passes
(
  Duration VARCHAR(25) NOT NULL,
  Transaction_Details INT NOT NULL,
  Membership_Type VARCHAR(20) NOT NULL,
  Fitness_Center_ID INT NOT NULL,
  PRIMARY KEY (Fitness_Center_ID),
  FOREIGN KEY (Fitness_Center_ID) REFERENCES Members(Fitness_Center_ID)
);

CREATE TABLE Non_Members
(
  Name VARCHAR(25) NOT NULL,
  Is_Instructor BOOLEAN,
  PRIMARY KEY (Name)
);

CREATE TABLE Camp
(
  Name_of_Camp VARCHAR(25) NOT NULL,
  Fitness_Center_ID INT NOT NULL,
  Instructor_ID INT,
  Instructor_Name VARCHAR(25),
  FOREIGN KEY (Fitness_Center_ID) REFERENCES Passes(Fitness_Center_ID),
  FOREIGN KEY (Instructor_ID) REFERENCES Members(Fitness_Center_ID),
  FOREIGN KEY (Instructor_Name) REFERENCES Non_Members(Name)
);

CREATE TABLE Merch
(
  Item_Name VARCHAR(25) NOT NULL,
  Transaction_Details INT NOT NULL,
  Name VARCHAR(25),
  Fitness_Center_ID INT,
  FOREIGN KEY (Name) REFERENCES Non_Members(Name),
  FOREIGN KEY (Fitness_Center_ID) REFERENCES Members(Fitness_Center_ID)
);

--Assumptions
--Only one instructor per camp.
--Membership type (worst to best): Steel, Platinum, Diamond

insert into Members (Name, Mailing_Address, Status, Contact_Information, Fitness_Center_ID, Membership_Type)
values ('Delores Abernathy', '240 Back Yonder Ave', 'FULL', '403-245-9420', 1, 'Diamond'),
('Teddy Flood', '248 Back Yonder Ave', 'FULL', '403-245-9420', 2, 'Steel'),
('William Black', '242 Real World St', 'Guest', '403-250-9543', 3, 'Steel'),
('Maeve Millary', '245 Back Yonder Ave', 'FULL', '403-224-9120', 4, 'Platinum'),
('Bernard Lowe', '248 Not Quite There Ave', 'Drop-In', '403-654-2325', 5, 'Diamond'),
('Theresa Cullen', '240 Damage Control Drive', 'Drop-In', '250-753-8239', 6, 'Diamond');

insert into Passes (Duration, Transaction_Details, Membership_Type, Fitness_Center_ID)
values ('Quarterly Pass', 123405, 'Diamond', 1),
('Yearly Pass', 432120, 'Steel', 2),
('Drop-In', 420569, 'Steel', 3),
('Quarterly Pass', 543323, 'Platinum', 4),
('10 Day Pass', 832903, 'Diamond', 5),
('Drop-In', 234854, 'Diamond', 6);

insert into Non_Members (Name, Is_Instructor)
values ('Charlotte Hale', FALSE),
('Logan', FALSE),
('Ashley Stubbs', FALSE),
('Robert Ford', TRUE),
('Felix Lutz', FALSE);

insert into Camp (Name_of_Camp, Fitness_Center_ID, Instructor_ID, Instructor_Name)
values ('Orientation', 2, 5, NULL),
('Orientation', 1, 5, NULL),
('Orientation', 4, 5, NULL),
('Company Meeting', 5, NULL, 'Robert Ford'),
('Company Meeting', 3, NULL, 'Robert Ford');

insert into Merch (Item_Name, Transaction_Details, Name, Fitness_Center_ID)
values ('Snapback Hat', 234050, 'Robert Ford', NULL),
('T-Shirt (Black)', 453782, NULL, 3),
('Commemorative Glass', 453782, NULL, 3),
('Yoga Pants', 345823, NULL, 5);