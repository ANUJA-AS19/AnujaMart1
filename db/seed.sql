INSERT INTO users (name, email, password_hash, role)
VALUES (
    'AnujaMart Admin',
    'admin@anujamart.com',
    '$argon2id$v=19$m=65536,t=2,p=1$XxX8mMbxjZfNlYzy/KMAug$URFl2WRl4xO7Kz2CIzdCbu744+IJ8uIVz0rhQV+0FnU',
    'ADMIN'
)
ON CONFLICT (email) DO NOTHING;
